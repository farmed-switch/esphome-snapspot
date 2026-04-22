

#include <mqtt.h>

enum MQTTErrors mqtt_sync(struct mqtt_client *client) {

    enum MQTTErrors err;
    int reconnecting = 0;
    MQTT_PAL_MUTEX_LOCK(&client->mutex);
    if (client->error != MQTT_ERROR_RECONNECTING && client->error != MQTT_OK && client->reconnect_callback != NULL) {
        client->reconnect_callback(client, &client->reconnect_state);

    } else {

        if (client->error == MQTT_ERROR_RECONNECTING) {
            reconnecting = 1;
            client->error = MQTT_OK;
        }
        MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
    }

    if (client->inspector_callback != NULL) {
        MQTT_PAL_MUTEX_LOCK(&client->mutex);
        err = client->inspector_callback(client);
        MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
        if (err != MQTT_OK) return err;
    }

    err = __mqtt_recv(client);
    if (err != MQTT_OK) return err;

    err = __mqtt_send(client);

    if (reconnecting && client->reconnect_callback != NULL) {
        MQTT_PAL_MUTEX_LOCK(&client->mutex);
        client->reconnect_callback(client, &client->reconnect_state);
    }

    return err;
}

uint16_t __mqtt_next_pid(struct mqtt_client *client) {
    int pid_exists = 0;
    if (client->pid_lfsr == 0) {
        client->pid_lfsr = 163u;
    }

    do {
        struct mqtt_queued_message *curr;
        unsigned lsb = client->pid_lfsr & 1;
        (client->pid_lfsr) >>= 1;
        if (lsb) {
            client->pid_lfsr ^= 0xB400u;
        }

        pid_exists = 0;
        for(curr = mqtt_mq_get(&(client->mq), 0); curr >= client->mq.queue_tail; --curr) {
            if (curr->packet_id == client->pid_lfsr) {
                pid_exists = 1;
                break;
            }
        }

    } while(pid_exists);
    return client->pid_lfsr;
}

enum MQTTErrors mqtt_init(struct mqtt_client *client,
               mqtt_pal_socket_handle sockfd,
               uint8_t *sendbuf, size_t sendbufsz,
               uint8_t *recvbuf, size_t recvbufsz,
               void (*publish_response_callback)(void** state,struct mqtt_response_publish *publish))
{
    if (client == NULL || sendbuf == NULL || recvbuf == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

    MQTT_PAL_MUTEX_INIT(&client->mutex);
    MQTT_PAL_MUTEX_LOCK(&client->mutex);

    client->socketfd = sockfd;

    mqtt_mq_init(&client->mq, sendbuf, sendbufsz);

    client->recv_buffer.mem_start = recvbuf;
    client->recv_buffer.mem_size = recvbufsz;
    client->recv_buffer.curr = client->recv_buffer.mem_start;
    client->recv_buffer.curr_sz = client->recv_buffer.mem_size;

    client->error = MQTT_ERROR_CONNECT_NOT_CALLED;
    client->response_timeout = 30;
    client->number_of_timeouts = 0;
    client->number_of_keep_alives = 0;
    client->typical_response_time = -1.0;
    client->publish_response_callback = publish_response_callback;
    client->pid_lfsr = 0;
    client->send_offset = 0;

    client->inspector_callback = NULL;
    client->reconnect_callback = NULL;
    client->reconnect_state = NULL;

    return MQTT_OK;
}

void mqtt_init_reconnect(struct mqtt_client *client,
                         void (*reconnect)(struct mqtt_client *, void**),
                         void *reconnect_state,
                         void (*publish_response_callback)(void** state, struct mqtt_response_publish *publish))
{

    MQTT_PAL_MUTEX_INIT(&client->mutex);

    client->socketfd = (mqtt_pal_socket_handle) -1;

    mqtt_mq_init(&client->mq, NULL, 0);

    client->recv_buffer.mem_start = NULL;
    client->recv_buffer.mem_size = 0;
    client->recv_buffer.curr = NULL;
    client->recv_buffer.curr_sz = 0;

    client->error = MQTT_ERROR_INITIAL_RECONNECT;
    client->response_timeout = 30;
    client->number_of_timeouts = 0;
    client->number_of_keep_alives = 0;
    client->typical_response_time = -1.0;
    client->publish_response_callback = publish_response_callback;
    client->send_offset = 0;

    client->inspector_callback = NULL;
    client->reconnect_callback = reconnect;
    client->reconnect_state = reconnect_state;
}

void mqtt_reinit(struct mqtt_client* client,
                 mqtt_pal_socket_handle socketfd,
                 uint8_t *sendbuf, size_t sendbufsz,
                 uint8_t *recvbuf, size_t recvbufsz)
{
    client->error = MQTT_ERROR_CONNECT_NOT_CALLED;
    client->socketfd = socketfd;

    mqtt_mq_init(&client->mq, sendbuf, sendbufsz);

    client->recv_buffer.mem_start = recvbuf;
    client->recv_buffer.mem_size = recvbufsz;
    client->recv_buffer.curr = client->recv_buffer.mem_start;
    client->recv_buffer.curr_sz = client->recv_buffer.mem_size;
}

#define MQTT_CLIENT_TRY_PACK(tmp, msg, client, pack_call, release)  \
    if (client->error < 0) {                                        \
        if (release) MQTT_PAL_MUTEX_UNLOCK(&client->mutex);         \
        return client->error;                                       \
    }                                                               \
    tmp = pack_call;                                                \
    if (tmp < 0) {                                                  \
        client->error = tmp;                                        \
        if (release) MQTT_PAL_MUTEX_UNLOCK(&client->mutex);         \
        return tmp;                                                 \
    } else if (tmp == 0) {                                          \
        mqtt_mq_clean(&client->mq);                                 \
        tmp = pack_call;                                            \
        if (tmp < 0) {                                              \
            client->error = tmp;                                    \
            if (release) MQTT_PAL_MUTEX_UNLOCK(&client->mutex);     \
            return tmp;                                             \
        } else if(tmp == 0) {                                       \
            client->error = MQTT_ERROR_SEND_BUFFER_IS_FULL;         \
            if (release) MQTT_PAL_MUTEX_UNLOCK(&client->mutex);     \
            return MQTT_ERROR_SEND_BUFFER_IS_FULL;                  \
        }                                                           \
    }                                                               \
    msg = mqtt_mq_register(&client->mq, tmp);                       \

enum MQTTErrors mqtt_connect(struct mqtt_client *client,
                     const char* client_id,
                     const char* will_topic,
                     const void* will_message,
                     size_t will_message_size,
                     const char* user_name,
                     const char* password,
                     uint8_t connect_flags,
                     uint16_t keep_alive)
{
    ssize_t rv;
    struct mqtt_queued_message *msg;

    client->keep_alive = keep_alive;
    if (client->error == MQTT_ERROR_CONNECT_NOT_CALLED) {
        client->error = MQTT_OK;
    }

    MQTT_CLIENT_TRY_PACK(rv, msg, client,
        mqtt_pack_connection_request(
            client->mq.curr, client->mq.curr_sz,
            client_id, will_topic, will_message,
            will_message_size,user_name, password,
            connect_flags, keep_alive
        ),
        1
    );

    msg->control_type = MQTT_CONTROL_CONNECT;

    MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
    return MQTT_OK;
}

enum MQTTErrors mqtt_publish(struct mqtt_client *client,
                     const char* topic_name,
                     const void* application_message,
                     size_t application_message_size,
                     uint8_t publish_flags)
{
    struct mqtt_queued_message *msg;
    ssize_t rv;
    uint16_t packet_id;
    MQTT_PAL_MUTEX_LOCK(&client->mutex);
    packet_id = __mqtt_next_pid(client);

    MQTT_CLIENT_TRY_PACK(
        rv, msg, client,
        mqtt_pack_publish_request(
            client->mq.curr, client->mq.curr_sz,
            topic_name,
            packet_id,
            application_message,
            application_message_size,
            publish_flags
        ),
        1
    );

    msg->control_type = MQTT_CONTROL_PUBLISH;
    msg->packet_id = packet_id;

    MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
    return MQTT_OK;
}

ssize_t __mqtt_puback(struct mqtt_client *client, uint16_t packet_id) {
    ssize_t rv;
    struct mqtt_queued_message *msg;

    MQTT_CLIENT_TRY_PACK(
        rv, msg, client,
        mqtt_pack_pubxxx_request(
            client->mq.curr, client->mq.curr_sz,
            MQTT_CONTROL_PUBACK,
            packet_id
        ),
        0
    );

    msg->control_type = MQTT_CONTROL_PUBACK;
    msg->packet_id = packet_id;

    return MQTT_OK;
}

ssize_t __mqtt_pubrec(struct mqtt_client *client, uint16_t packet_id) {
    ssize_t rv;
    struct mqtt_queued_message *msg;

    MQTT_CLIENT_TRY_PACK(
        rv, msg, client,
        mqtt_pack_pubxxx_request(
            client->mq.curr, client->mq.curr_sz,
            MQTT_CONTROL_PUBREC,
            packet_id
        ),
        0
    );

    msg->control_type = MQTT_CONTROL_PUBREC;
    msg->packet_id = packet_id;

    return MQTT_OK;
}

ssize_t __mqtt_pubrel(struct mqtt_client *client, uint16_t packet_id) {
    ssize_t rv;
    struct mqtt_queued_message *msg;

    MQTT_CLIENT_TRY_PACK(
        rv, msg, client,
        mqtt_pack_pubxxx_request(
            client->mq.curr, client->mq.curr_sz,
            MQTT_CONTROL_PUBREL,
            packet_id
        ),
        0
    );

    msg->control_type = MQTT_CONTROL_PUBREL;
    msg->packet_id = packet_id;

    return MQTT_OK;
}

ssize_t __mqtt_pubcomp(struct mqtt_client *client, uint16_t packet_id) {
    ssize_t rv;
    struct mqtt_queued_message *msg;

    MQTT_CLIENT_TRY_PACK(
        rv, msg, client,
        mqtt_pack_pubxxx_request(
            client->mq.curr, client->mq.curr_sz,
            MQTT_CONTROL_PUBCOMP,
            packet_id
        ),
        0
    );

    msg->control_type = MQTT_CONTROL_PUBCOMP;
    msg->packet_id = packet_id;

    return MQTT_OK;
}

enum MQTTErrors mqtt_subscribe(struct mqtt_client *client,
                       const char* topic_name,
                       int max_qos_level)
{
    ssize_t rv;
    uint16_t packet_id;
    struct mqtt_queued_message *msg;
    MQTT_PAL_MUTEX_LOCK(&client->mutex);
    packet_id = __mqtt_next_pid(client);

    MQTT_CLIENT_TRY_PACK(
        rv, msg, client,
        mqtt_pack_subscribe_request(
            client->mq.curr, client->mq.curr_sz,
            packet_id,
            topic_name,
            max_qos_level,
            (const char*)NULL
        ),
        1
    );

    msg->control_type = MQTT_CONTROL_SUBSCRIBE;
    msg->packet_id = packet_id;

    MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
    return MQTT_OK;
}

enum MQTTErrors mqtt_unsubscribe(struct mqtt_client *client,
                         const char* topic_name)
{
    uint16_t packet_id = __mqtt_next_pid(client);
    ssize_t rv;
    struct mqtt_queued_message *msg;
    MQTT_PAL_MUTEX_LOCK(&client->mutex);

    MQTT_CLIENT_TRY_PACK(
        rv, msg, client,
        mqtt_pack_unsubscribe_request(
            client->mq.curr, client->mq.curr_sz,
            packet_id,
            topic_name,
            (const char*)NULL
        ),
        1
    );

    msg->control_type = MQTT_CONTROL_UNSUBSCRIBE;
    msg->packet_id = packet_id;

    MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
    return MQTT_OK;
}

enum MQTTErrors mqtt_ping(struct mqtt_client *client) {
    enum MQTTErrors rv;
    MQTT_PAL_MUTEX_LOCK(&client->mutex);
    rv = __mqtt_ping(client);
    MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
    return rv;
}

enum MQTTErrors __mqtt_ping(struct mqtt_client *client)
{
    ssize_t rv;
    struct mqtt_queued_message *msg;

    MQTT_CLIENT_TRY_PACK(
        rv, msg, client,
        mqtt_pack_ping_request(
            client->mq.curr, client->mq.curr_sz
        ),
        0
    );

    msg->control_type = MQTT_CONTROL_PINGREQ;

    return MQTT_OK;
}

enum MQTTErrors mqtt_reconnect(struct mqtt_client *client)
{
    enum MQTTErrors err = mqtt_disconnect(client);

    if (err == MQTT_OK) {
        MQTT_PAL_MUTEX_LOCK(&client->mutex);
        client->error = MQTT_ERROR_RECONNECTING;
        MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
    }
    return err;
}

enum MQTTErrors mqtt_disconnect(struct mqtt_client *client)
{
    ssize_t rv;
    struct mqtt_queued_message *msg;
    MQTT_PAL_MUTEX_LOCK(&client->mutex);

    MQTT_CLIENT_TRY_PACK(
        rv, msg, client,
        mqtt_pack_disconnect(
            client->mq.curr, client->mq.curr_sz
        ),
        1
    );

    msg->control_type = MQTT_CONTROL_DISCONNECT;

    MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
    return MQTT_OK;
}

ssize_t __mqtt_send(struct mqtt_client *client)
{
    uint8_t inspected;
    ssize_t len;
    int inflight_qos2 = 0;
    int i = 0;

    MQTT_PAL_MUTEX_LOCK(&client->mutex);

    if (client->error < 0 && client->error != MQTT_ERROR_SEND_BUFFER_IS_FULL) {
        MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
        return client->error;
    }

    len = mqtt_mq_length(&client->mq);
    for(; i < len; ++i) {
        struct mqtt_queued_message *msg = mqtt_mq_get(&client->mq, i);
        int resend = 0;
        if (msg->state == MQTT_QUEUED_UNSENT) {

            resend = 1;
        } else if (msg->state == MQTT_QUEUED_AWAITING_ACK) {

            if (MQTT_PAL_TIME() > msg->time_sent + client->response_timeout) {
                resend = 1;
                client->number_of_timeouts += 1;
                client->send_offset = 0;
            }
        }

        if (msg->control_type == MQTT_CONTROL_PUBLISH
            && (msg->state == MQTT_QUEUED_UNSENT || msg->state == MQTT_QUEUED_AWAITING_ACK))
        {
            inspected = 0x03 & ((msg->start[0]) >> 1);
            if (inspected == 2) {
                if (inflight_qos2) {
                    resend = 0;
                }
                inflight_qos2 = 1;
            }
        }

        if (!resend) {
            continue;
        }

        {
          ssize_t tmp = mqtt_pal_sendall(client->socketfd, msg->start + client->send_offset, msg->size - client->send_offset, 0);
          if (tmp < 0) {
            client->error = tmp;
            MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
            return tmp;
          } else {
            client->send_offset += tmp;
            if(client->send_offset < msg->size) {

              break;
            } else {

              client->send_offset = 0;
            }

          }

        }

        client->time_of_last_send = MQTT_PAL_TIME();
        msg->time_sent = client->time_of_last_send;

        switch (msg->control_type) {
        case MQTT_CONTROL_PUBACK:
        case MQTT_CONTROL_PUBCOMP:
        case MQTT_CONTROL_DISCONNECT:
            msg->state = MQTT_QUEUED_COMPLETE;
            break;
        case MQTT_CONTROL_PUBLISH:
            inspected = ( MQTT_PUBLISH_QOS_MASK & (msg->start[0]) ) >> 1;
            if (inspected == 0) {
                msg->state = MQTT_QUEUED_COMPLETE;
            } else if (inspected == 1) {
                msg->state = MQTT_QUEUED_AWAITING_ACK;

                msg->start[0] |= MQTT_PUBLISH_DUP;
            } else {
                msg->state = MQTT_QUEUED_AWAITING_ACK;
            }
            break;
        case MQTT_CONTROL_CONNECT:
        case MQTT_CONTROL_PUBREC:
        case MQTT_CONTROL_PUBREL:
        case MQTT_CONTROL_SUBSCRIBE:
        case MQTT_CONTROL_UNSUBSCRIBE:
        case MQTT_CONTROL_PINGREQ:
            msg->state = MQTT_QUEUED_AWAITING_ACK;
            break;
        default:
            client->error = MQTT_ERROR_MALFORMED_REQUEST;
            MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
            return MQTT_ERROR_MALFORMED_REQUEST;
        }
    }

    {
        mqtt_pal_time_t keep_alive_timeout = client->time_of_last_send + (mqtt_pal_time_t)((float)(client->keep_alive) * 0.75);
        if (MQTT_PAL_TIME() > keep_alive_timeout) {
          ssize_t rv = __mqtt_ping(client);
          if (rv != MQTT_OK) {
            client->error = rv;
            MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
            return rv;
          }
        }
    }

    MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
    return MQTT_OK;
}

ssize_t __mqtt_recv(struct mqtt_client *client)
{
    struct mqtt_response response;
    ssize_t mqtt_recv_ret = MQTT_OK;
    MQTT_PAL_MUTEX_LOCK(&client->mutex);

    while(mqtt_recv_ret == MQTT_OK) {

        ssize_t rv, consumed;
        struct mqtt_queued_message *msg = NULL;

        rv = mqtt_pal_recvall(client->socketfd, client->recv_buffer.curr, client->recv_buffer.curr_sz, 0);
        if (rv < 0) {

            client->error = rv;
            MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
            return rv;
        } else {
            client->recv_buffer.curr += rv;
            client->recv_buffer.curr_sz -= rv;
        }

        consumed = mqtt_unpack_response(&response, client->recv_buffer.mem_start, client->recv_buffer.curr - client->recv_buffer.mem_start);

        if (consumed < 0) {
            client->error = consumed;
            MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
            return consumed;
        } else if (consumed == 0) {

            if (client->recv_buffer.curr_sz == 0) {
                client->error = MQTT_ERROR_RECV_BUFFER_TOO_SMALL;
                MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
                return MQTT_ERROR_RECV_BUFFER_TOO_SMALL;
            }

            MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
            return MQTT_OK;
        }

        switch (response.fixed_header.control_type) {
            case MQTT_CONTROL_CONNACK:

                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_CONNECT, NULL);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    mqtt_recv_ret = MQTT_ERROR_ACK_OF_UNKNOWN;
                    break;
                }
                msg->state = MQTT_QUEUED_COMPLETE;

                client->typical_response_time = (double) (MQTT_PAL_TIME() - msg->time_sent);

                if (response.decoded.connack.return_code != MQTT_CONNACK_ACCEPTED) {
                    if (response.decoded.connack.return_code == MQTT_CONNACK_REFUSED_IDENTIFIER_REJECTED) {
                        client->error = MQTT_ERROR_CONNECT_CLIENT_ID_REFUSED;
                        mqtt_recv_ret = MQTT_ERROR_CONNECT_CLIENT_ID_REFUSED;
                    } else {
                        client->error = MQTT_ERROR_CONNECTION_REFUSED;
                        mqtt_recv_ret = MQTT_ERROR_CONNECTION_REFUSED;
                    }
                    break;
                }
                break;
            case MQTT_CONTROL_PUBLISH:

                if (response.decoded.publish.qos_level == 1) {
                    rv = __mqtt_puback(client, response.decoded.publish.packet_id);
                    if (rv != MQTT_OK) {
                        client->error = rv;
                        mqtt_recv_ret = rv;
                        break;
                    }
                } else if (response.decoded.publish.qos_level == 2) {

                    if (mqtt_mq_find(&client->mq, MQTT_CONTROL_PUBREC, &response.decoded.publish.packet_id) != NULL) {
                        break;
                    }

                    rv = __mqtt_pubrec(client, response.decoded.publish.packet_id);
                    if (rv != MQTT_OK) {
                        client->error = rv;
                        mqtt_recv_ret = rv;
                        break;
                    }
                }

                client->publish_response_callback(&client->publish_response_callback_state, &response.decoded.publish);
                break;
            case MQTT_CONTROL_PUBACK:

                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_PUBLISH, &response.decoded.puback.packet_id);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    mqtt_recv_ret = MQTT_ERROR_ACK_OF_UNKNOWN;
                    break;
                }
                msg->state = MQTT_QUEUED_COMPLETE;

                client->typical_response_time = 0.875 * (client->typical_response_time) + 0.125 * (double) (MQTT_PAL_TIME() - msg->time_sent);
                break;
            case MQTT_CONTROL_PUBREC:

                if (mqtt_mq_find(&client->mq, MQTT_CONTROL_PUBREL, &response.decoded.pubrec.packet_id) != NULL) {
                    break;
                }

                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_PUBLISH, &response.decoded.pubrec.packet_id);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    mqtt_recv_ret = MQTT_ERROR_ACK_OF_UNKNOWN;
                    break;
                }
                msg->state = MQTT_QUEUED_COMPLETE;

                client->typical_response_time = 0.875 * (client->typical_response_time) + 0.125 * (double) (MQTT_PAL_TIME() - msg->time_sent);

                rv = __mqtt_pubrel(client, response.decoded.pubrec.packet_id);
                if (rv != MQTT_OK) {
                    client->error = rv;
                    mqtt_recv_ret = rv;
                    break;
                }
                break;
            case MQTT_CONTROL_PUBREL:

                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_PUBREC, &response.decoded.pubrel.packet_id);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    mqtt_recv_ret = MQTT_ERROR_ACK_OF_UNKNOWN;
                    break;
                }
                msg->state = MQTT_QUEUED_COMPLETE;

                client->typical_response_time = 0.875 * (client->typical_response_time) + 0.125 * (double) (MQTT_PAL_TIME() - msg->time_sent);

                rv = __mqtt_pubcomp(client, response.decoded.pubrec.packet_id);
                if (rv != MQTT_OK) {
                    client->error = rv;
                    mqtt_recv_ret = rv;
                    break;
                }
                break;
            case MQTT_CONTROL_PUBCOMP:

                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_PUBREL, &response.decoded.pubcomp.packet_id);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    mqtt_recv_ret = MQTT_ERROR_ACK_OF_UNKNOWN;
                    break;
                }
                msg->state = MQTT_QUEUED_COMPLETE;

                client->typical_response_time = 0.875 * (client->typical_response_time) + 0.125 * (double) (MQTT_PAL_TIME() - msg->time_sent);
                break;
            case MQTT_CONTROL_SUBACK:

                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_SUBSCRIBE, &response.decoded.suback.packet_id);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    mqtt_recv_ret = MQTT_ERROR_ACK_OF_UNKNOWN;
                    break;
                }
                msg->state = MQTT_QUEUED_COMPLETE;

                client->typical_response_time = 0.875 * (client->typical_response_time) + 0.125 * (double) (MQTT_PAL_TIME() - msg->time_sent);

                if (response.decoded.suback.return_codes[0] == MQTT_SUBACK_FAILURE) {
                    client->error = MQTT_ERROR_SUBSCRIBE_FAILED;
                    mqtt_recv_ret = MQTT_ERROR_SUBSCRIBE_FAILED;
                    break;
                }
                break;
            case MQTT_CONTROL_UNSUBACK:

                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_UNSUBSCRIBE, &response.decoded.unsuback.packet_id);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    mqtt_recv_ret = MQTT_ERROR_ACK_OF_UNKNOWN;
                    break;
                }
                msg->state = MQTT_QUEUED_COMPLETE;

                client->typical_response_time = 0.875 * (client->typical_response_time) + 0.125 * (double) (MQTT_PAL_TIME() - msg->time_sent);
                break;
            case MQTT_CONTROL_PINGRESP:

                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_PINGREQ, NULL);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    mqtt_recv_ret = MQTT_ERROR_ACK_OF_UNKNOWN;
                    break;
                }
                msg->state = MQTT_QUEUED_COMPLETE;

                client->typical_response_time = 0.875 * (client->typical_response_time) + 0.125 * (double) (MQTT_PAL_TIME() - msg->time_sent);
                break;
            default:
                client->error = MQTT_ERROR_MALFORMED_RESPONSE;
                mqtt_recv_ret = MQTT_ERROR_MALFORMED_RESPONSE;
                break;
        }
        {

          void* dest = (unsigned char*)client->recv_buffer.mem_start;
          void* src  = (unsigned char*)client->recv_buffer.mem_start + consumed;
          size_t n = client->recv_buffer.curr - client->recv_buffer.mem_start - consumed;
          memmove(dest, src, n);
          client->recv_buffer.curr -= consumed;
          client->recv_buffer.curr_sz += consumed;
        }
    }

    MQTT_PAL_MUTEX_UNLOCK(&client->mutex);
    return mqtt_recv_ret;
}

#define MQTT_BITFIELD_RULE_VIOLOATION(bitfield, rule_value, rule_mask) ((bitfield ^ rule_value) & rule_mask)

struct {
    const uint8_t control_type_is_valid[16];
    const uint8_t required_flags[16];
    const uint8_t mask_required_flags[16];
} mqtt_fixed_header_rules = {
    {
        0x00,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x00
    },
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x02,
        0x00,
        0x02,
        0x00,
        0x02,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00
    },
    {
        0x00,
        0x0F,
        0x0F,
        0x00,
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x0F,
        0x00
    }
};

static ssize_t mqtt_fixed_header_rule_violation(const struct mqtt_fixed_header *fixed_header) {
    uint8_t control_type;
    uint8_t control_flags;
    uint8_t required_flags;
    uint8_t mask_required_flags;

    control_type = fixed_header->control_type;
    control_flags = fixed_header->control_flags;
    required_flags = mqtt_fixed_header_rules.required_flags[control_type];
    mask_required_flags = mqtt_fixed_header_rules.mask_required_flags[control_type];

    if (!mqtt_fixed_header_rules.control_type_is_valid[control_type]) {
        return MQTT_ERROR_CONTROL_FORBIDDEN_TYPE;
    }

    if(MQTT_BITFIELD_RULE_VIOLOATION(control_flags, required_flags, mask_required_flags)) {
        return MQTT_ERROR_CONTROL_INVALID_FLAGS;
    }

    return 0;
}

ssize_t mqtt_unpack_fixed_header(struct mqtt_response *response, const uint8_t *buf, size_t bufsz) {
    struct mqtt_fixed_header *fixed_header;
    const uint8_t *start = buf;
    int lshift;
    ssize_t errcode;

    if (response == NULL || buf == NULL) {
        return MQTT_ERROR_NULLPTR;
    }
    fixed_header = &(response->fixed_header);

    if (bufsz == 0) return 0;

    fixed_header->control_type  = *buf >> 4;
    fixed_header->control_flags = *buf & 0x0F;

    fixed_header->remaining_length = 0;

    lshift = 0;
    do {

        if(lshift == 28)
            return MQTT_ERROR_INVALID_REMAINING_LENGTH;

        --bufsz;
        ++buf;
        if (bufsz == 0) return 0;

        fixed_header->remaining_length += (*buf & 0x7F) << lshift;
        lshift += 7;
    } while(*buf & 0x80);

    --bufsz;
    ++buf;

    errcode = mqtt_fixed_header_rule_violation(fixed_header);
    if (errcode) {
        return errcode;
    }

    if (bufsz < fixed_header->remaining_length) {
        return 0;
    }

    return buf - start;
}

ssize_t mqtt_pack_fixed_header(uint8_t *buf, size_t bufsz, const struct mqtt_fixed_header *fixed_header) {
    const uint8_t *start = buf;
    ssize_t errcode;
    uint32_t remaining_length;

    if (fixed_header == NULL || buf == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

    errcode = mqtt_fixed_header_rule_violation(fixed_header);
    if (errcode) {
        return errcode;
    }

    if (bufsz == 0) return 0;

    *buf =  (((uint8_t) fixed_header->control_type) << 4) & 0xF0;
    *buf |= ((uint8_t) fixed_header->control_flags)       & 0x0F;

    remaining_length = fixed_header->remaining_length;

    if(remaining_length >= 256*1024*1024)
        return MQTT_ERROR_INVALID_REMAINING_LENGTH;

    do {

        --bufsz;
        ++buf;
        if (bufsz == 0) return 0;

        *buf  = remaining_length & 0x7F;
        if(remaining_length > 127) *buf |= 0x80;
        remaining_length = remaining_length >> 7;
    } while(*buf & 0x80);

    --bufsz;
    ++buf;

    if (bufsz < fixed_header->remaining_length) {
        return 0;
    }

    return buf - start;
}

ssize_t mqtt_pack_connection_request(uint8_t* buf, size_t bufsz,
                                     const char* client_id,
                                     const char* will_topic,
                                     const void* will_message,
                                     size_t will_message_size,
                                     const char* user_name,
                                     const char* password,
                                     uint8_t connect_flags,
                                     uint16_t keep_alive)
{
    struct mqtt_fixed_header fixed_header;
    size_t remaining_length;
    const uint8_t *const start = buf;
    ssize_t rv;

    fixed_header.control_type = MQTT_CONTROL_CONNECT;
    fixed_header.control_flags = 0x00;

    connect_flags = connect_flags & ~MQTT_CONNECT_RESERVED;
    remaining_length = 10;

    if (client_id == NULL) {
        client_id = "";
    }

    if (client_id[0] == '\0' && !(connect_flags & MQTT_CONNECT_CLEAN_SESSION)) {
        return MQTT_ERROR_CLEAN_SESSION_IS_REQUIRED;
    }

    remaining_length += __mqtt_packed_cstrlen(client_id);

    if (will_topic != NULL) {
        uint8_t temp;

        connect_flags |= MQTT_CONNECT_WILL_FLAG;
        remaining_length += __mqtt_packed_cstrlen(will_topic);

        if (will_message == NULL) {

            return MQTT_ERROR_CONNECT_NULL_WILL_MESSAGE;
        }
        remaining_length += 2 + will_message_size;

        temp = connect_flags & 0x18;
        if (temp == 0x18) {

            return MQTT_ERROR_CONNECT_FORBIDDEN_WILL_QOS;
        }
    } else {

        connect_flags &= ~MQTT_CONNECT_WILL_FLAG;
        connect_flags &= ~0x18;
        connect_flags &= ~MQTT_CONNECT_WILL_RETAIN;
    }

    if (user_name != NULL) {

        connect_flags |= MQTT_CONNECT_USER_NAME;
        remaining_length += __mqtt_packed_cstrlen(user_name);
    } else {
        connect_flags &= ~MQTT_CONNECT_USER_NAME;
    }

    if (password != NULL) {

        connect_flags |= MQTT_CONNECT_PASSWORD;
        remaining_length += __mqtt_packed_cstrlen(password);
    } else {
        connect_flags &= ~MQTT_CONNECT_PASSWORD;
    }

    fixed_header.remaining_length = remaining_length;

    rv = mqtt_pack_fixed_header(buf, bufsz, &fixed_header);
    if (rv <= 0) {

        return rv;
    }
    buf += rv;
    bufsz -= rv;

    if (bufsz < fixed_header.remaining_length) {
        return 0;
    }

    *buf++ = 0x00;
    *buf++ = 0x04;
    *buf++ = (uint8_t) 'M';
    *buf++ = (uint8_t) 'Q';
    *buf++ = (uint8_t) 'T';
    *buf++ = (uint8_t) 'T';
    *buf++ = MQTT_PROTOCOL_LEVEL;
    *buf++ = connect_flags;
    buf += __mqtt_pack_uint16(buf, keep_alive);

    buf += __mqtt_pack_str(buf, client_id);
    if (connect_flags & MQTT_CONNECT_WILL_FLAG) {
        buf += __mqtt_pack_str(buf, will_topic);
        buf += __mqtt_pack_uint16(buf, (uint16_t)will_message_size);
        memcpy(buf, will_message, will_message_size);
        buf += will_message_size;
    }
    if (connect_flags & MQTT_CONNECT_USER_NAME) {
        buf += __mqtt_pack_str(buf, user_name);
    }
    if (connect_flags & MQTT_CONNECT_PASSWORD) {
        buf += __mqtt_pack_str(buf, password);
    }

    return buf - start;
}

ssize_t mqtt_unpack_connack_response(struct mqtt_response *mqtt_response, const uint8_t *buf) {
    const uint8_t *const start = buf;
    struct mqtt_response_connack *response;

    if (mqtt_response->fixed_header.remaining_length != 2) {
        return MQTT_ERROR_MALFORMED_RESPONSE;
    }

    response = &(mqtt_response->decoded.connack);

    if (*buf & 0xFE) {

        return MQTT_ERROR_CONNACK_FORBIDDEN_FLAGS;
    } else {
        response->session_present_flag = *buf++;
    }

    if (*buf > 5u) {

        return MQTT_ERROR_CONNACK_FORBIDDEN_CODE;
    } else {
        response->return_code = (enum MQTTConnackReturnCode) *buf++;
    }
    return buf - start;
}

ssize_t mqtt_pack_disconnect(uint8_t *buf, size_t bufsz) {
    struct mqtt_fixed_header fixed_header;
    fixed_header.control_type = MQTT_CONTROL_DISCONNECT;
    fixed_header.control_flags = 0;
    fixed_header.remaining_length = 0;
    return mqtt_pack_fixed_header(buf, bufsz, &fixed_header);
}

ssize_t mqtt_pack_ping_request(uint8_t *buf, size_t bufsz) {
    struct mqtt_fixed_header fixed_header;
    fixed_header.control_type = MQTT_CONTROL_PINGREQ;
    fixed_header.control_flags = 0;
    fixed_header.remaining_length = 0;
    return mqtt_pack_fixed_header(buf, bufsz, &fixed_header);
}

ssize_t mqtt_pack_publish_request(uint8_t *buf, size_t bufsz,
                                  const char* topic_name,
                                  uint16_t packet_id,
                                  const void* application_message,
                                  size_t application_message_size,
                                  uint8_t publish_flags)
{
    const uint8_t *const start = buf;
    ssize_t rv;
    struct mqtt_fixed_header fixed_header;
    uint32_t remaining_length;
    uint8_t inspected_qos;

    if(buf == NULL || topic_name == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

    inspected_qos = (publish_flags & MQTT_PUBLISH_QOS_MASK) >> 1;

    fixed_header.control_type = MQTT_CONTROL_PUBLISH;

    remaining_length = (uint32_t)__mqtt_packed_cstrlen(topic_name);
    if (inspected_qos > 0) {
        remaining_length += 2;
    }
    remaining_length += (uint32_t)application_message_size;
    fixed_header.remaining_length = remaining_length;

    if (inspected_qos == 0) {
        publish_flags &= ~MQTT_PUBLISH_DUP;
    }

    if (inspected_qos == 3) {
        return MQTT_ERROR_PUBLISH_FORBIDDEN_QOS;
    }
    fixed_header.control_flags = publish_flags;

    rv = mqtt_pack_fixed_header(buf, bufsz, &fixed_header);
    if (rv <= 0) {

        return rv;
    }
    buf += rv;
    bufsz -= rv;

    if (bufsz < remaining_length) {
        return 0;
    }

    buf += __mqtt_pack_str(buf, topic_name);
    if (inspected_qos > 0) {
        buf += __mqtt_pack_uint16(buf, packet_id);
    }

    memcpy(buf, application_message, application_message_size);
    buf += application_message_size;

    return buf - start;
}

ssize_t mqtt_unpack_publish_response(struct mqtt_response *mqtt_response, const uint8_t *buf)
{
    const uint8_t *const start = buf;
    struct mqtt_fixed_header *fixed_header;
    struct mqtt_response_publish *response;

    fixed_header = &(mqtt_response->fixed_header);
    response = &(mqtt_response->decoded.publish);

    response->dup_flag = (fixed_header->control_flags & MQTT_PUBLISH_DUP) >> 3;
    response->qos_level = (fixed_header->control_flags & MQTT_PUBLISH_QOS_MASK) >> 1;
    response->retain_flag = fixed_header->control_flags & MQTT_PUBLISH_RETAIN;

    if (mqtt_response->fixed_header.remaining_length < 4) {
        return MQTT_ERROR_MALFORMED_RESPONSE;
    }

    response->topic_name_size = __mqtt_unpack_uint16(buf);
    buf += 2;
    response->topic_name = buf;
    buf += response->topic_name_size;

    if (response->qos_level > 0) {
        response->packet_id = __mqtt_unpack_uint16(buf);
        buf += 2;
    }

    response->application_message = buf;
    if (response->qos_level == 0) {
        response->application_message_size = fixed_header->remaining_length - response->topic_name_size - 2;
    } else {
        response->application_message_size = fixed_header->remaining_length - response->topic_name_size - 4;
    }
    buf += response->application_message_size;

    return buf - start;
}

ssize_t mqtt_pack_pubxxx_request(uint8_t *buf, size_t bufsz,
                                 enum MQTTControlPacketType control_type,
                                 uint16_t packet_id)
{
    const uint8_t *const start = buf;
    struct mqtt_fixed_header fixed_header;
    ssize_t rv;
    if (buf == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

    fixed_header.control_type = control_type;
    if (control_type == MQTT_CONTROL_PUBREL) {
        fixed_header.control_flags = 0x02;
    } else {
        fixed_header.control_flags = 0;
    }
    fixed_header.remaining_length = 2;
    rv = mqtt_pack_fixed_header(buf, bufsz, &fixed_header);
    if (rv <= 0) {
        return rv;
    }
    buf += rv;
    bufsz -= rv;

    if (bufsz < fixed_header.remaining_length) {
        return 0;
    }

    buf += __mqtt_pack_uint16(buf, packet_id);

    return buf - start;
}

ssize_t mqtt_unpack_pubxxx_response(struct mqtt_response *mqtt_response, const uint8_t *buf)
{
    const uint8_t *const start = buf;
    uint16_t packet_id;

    if (mqtt_response->fixed_header.remaining_length != 2) {
        return MQTT_ERROR_MALFORMED_RESPONSE;
    }

    packet_id = __mqtt_unpack_uint16(buf);
    buf += 2;

    if (mqtt_response->fixed_header.control_type == MQTT_CONTROL_PUBACK) {
        mqtt_response->decoded.puback.packet_id = packet_id;
    } else if (mqtt_response->fixed_header.control_type == MQTT_CONTROL_PUBREC) {
        mqtt_response->decoded.pubrec.packet_id = packet_id;
    } else if (mqtt_response->fixed_header.control_type == MQTT_CONTROL_PUBREL) {
        mqtt_response->decoded.pubrel.packet_id = packet_id;
    } else {
        mqtt_response->decoded.pubcomp.packet_id = packet_id;
    }

    return buf - start;
}

ssize_t mqtt_unpack_suback_response (struct mqtt_response *mqtt_response, const uint8_t *buf) {
    const uint8_t *const start = buf;
    uint32_t remaining_length = mqtt_response->fixed_header.remaining_length;

    if (remaining_length < 3) {
        return MQTT_ERROR_MALFORMED_RESPONSE;
    }

    mqtt_response->decoded.suback.packet_id = __mqtt_unpack_uint16(buf);
    buf += 2;
    remaining_length -= 2;

    mqtt_response->decoded.suback.num_return_codes = (size_t) remaining_length;
    mqtt_response->decoded.suback.return_codes = buf;
    buf += remaining_length;

    return buf - start;
}

ssize_t mqtt_pack_subscribe_request(uint8_t *buf, size_t bufsz, unsigned int packet_id, ...) {
    va_list args;
    const uint8_t *const start = buf;
    ssize_t rv;
    struct mqtt_fixed_header fixed_header;
    unsigned int num_subs = 0;
    unsigned int i;
    const char *topic[MQTT_SUBSCRIBE_REQUEST_MAX_NUM_TOPICS];
    uint8_t max_qos[MQTT_SUBSCRIBE_REQUEST_MAX_NUM_TOPICS];

    va_start(args, packet_id);
    while(1) {
        topic[num_subs] = va_arg(args, const char*);
        if (topic[num_subs] == NULL) {

            break;
        }

        max_qos[num_subs] = (uint8_t) va_arg(args, unsigned int);

        ++num_subs;
        if (num_subs >= MQTT_SUBSCRIBE_REQUEST_MAX_NUM_TOPICS) {
            va_end(args);
            return MQTT_ERROR_SUBSCRIBE_TOO_MANY_TOPICS;
        }
    }
    va_end(args);

    fixed_header.control_type = MQTT_CONTROL_SUBSCRIBE;
    fixed_header.control_flags = 2u;
    fixed_header.remaining_length = 2u;
    for(i = 0; i < num_subs; ++i) {

        fixed_header.remaining_length += __mqtt_packed_cstrlen(topic[i]) + 1;
    }

    rv = mqtt_pack_fixed_header(buf, bufsz, &fixed_header);
    if (rv <= 0) {
        return rv;
    }
    buf += rv;
    bufsz -= rv;

    if (bufsz < fixed_header.remaining_length) {
        return 0;
    }

    buf += __mqtt_pack_uint16(buf, packet_id);

    for(i = 0; i < num_subs; ++i) {
        buf += __mqtt_pack_str(buf, topic[i]);
        *buf++ = max_qos[i];
    }

    return buf - start;
}

ssize_t mqtt_unpack_unsuback_response(struct mqtt_response *mqtt_response, const uint8_t *buf)
{
    const uint8_t *const start = buf;

    if (mqtt_response->fixed_header.remaining_length != 2) {
        return MQTT_ERROR_MALFORMED_RESPONSE;
    }

    mqtt_response->decoded.unsuback.packet_id = __mqtt_unpack_uint16(buf);
    buf += 2;

    return buf - start;
}

ssize_t mqtt_pack_unsubscribe_request(uint8_t *buf, size_t bufsz, unsigned int packet_id, ...) {
    va_list args;
    const uint8_t *const start = buf;
    ssize_t rv;
    struct mqtt_fixed_header fixed_header;
    unsigned int num_subs = 0;
    unsigned int i;
    const char *topic[MQTT_UNSUBSCRIBE_REQUEST_MAX_NUM_TOPICS];

    va_start(args, packet_id);
    while(1) {
        topic[num_subs] = va_arg(args, const char*);
        if (topic[num_subs] == NULL) {

            break;
        }

        ++num_subs;
        if (num_subs >= MQTT_UNSUBSCRIBE_REQUEST_MAX_NUM_TOPICS) {
            va_end(args);
            return MQTT_ERROR_UNSUBSCRIBE_TOO_MANY_TOPICS;
        }
    }
    va_end(args);

    fixed_header.control_type = MQTT_CONTROL_UNSUBSCRIBE;
    fixed_header.control_flags = 2u;
    fixed_header.remaining_length = 2u;
    for(i = 0; i < num_subs; ++i) {

        fixed_header.remaining_length += __mqtt_packed_cstrlen(topic[i]);
    }

    rv = mqtt_pack_fixed_header(buf, bufsz, &fixed_header);
    if (rv <= 0) {
        return rv;
    }
    buf += rv;
    bufsz -= rv;

    if (bufsz < fixed_header.remaining_length) {
        return 0;
    }

    buf += __mqtt_pack_uint16(buf, packet_id);

    for(i = 0; i < num_subs; ++i) {
        buf += __mqtt_pack_str(buf, topic[i]);
    }

    return buf - start;
}

void mqtt_mq_init(struct mqtt_message_queue *mq, void *buf, size_t bufsz)
{
    if(buf != NULL)
    {
        mq->mem_start = buf;
        mq->mem_end = (unsigned char*)buf + bufsz;
        mq->curr = buf;
        mq->queue_tail = mq->mem_end;
        mq->curr_sz = mqtt_mq_currsz(mq);
    }
}

struct mqtt_queued_message* mqtt_mq_register(struct mqtt_message_queue *mq, size_t nbytes)
{

    --(mq->queue_tail);
    mq->queue_tail->start = mq->curr;
    mq->queue_tail->size = nbytes;
    mq->queue_tail->state = MQTT_QUEUED_UNSENT;

    mq->curr += nbytes;
    mq->curr_sz = mqtt_mq_currsz(mq);

    return mq->queue_tail;
}

void mqtt_mq_clean(struct mqtt_message_queue *mq) {
    struct mqtt_queued_message *new_head;

    for(new_head = mqtt_mq_get(mq, 0); new_head >= mq->queue_tail; --new_head) {
        if (new_head->state != MQTT_QUEUED_COMPLETE) break;
    }

    if (new_head < mq->queue_tail) {
        mq->curr = mq->mem_start;
        mq->queue_tail = mq->mem_end;
        mq->curr_sz = mqtt_mq_currsz(mq);
        return;
    } else if (new_head == mqtt_mq_get(mq, 0)) {

        return;
    }

    {
        size_t n = mq->curr - new_head->start;
        size_t removing = new_head->start - (uint8_t*) mq->mem_start;
        memmove(mq->mem_start, new_head->start, n);
        mq->curr = (unsigned char*)mq->mem_start + n;

        {
            ssize_t new_tail_idx = new_head - mq->queue_tail;
            memmove(mqtt_mq_get(mq, new_tail_idx), mq->queue_tail, sizeof(struct mqtt_queued_message) * (new_tail_idx + 1));
            mq->queue_tail = mqtt_mq_get(mq, new_tail_idx);

            {

                ssize_t i = 0;
                for(; i < new_tail_idx + 1; ++i) {
                    mqtt_mq_get(mq, i)->start -= removing;
                }
            }
        }
    }

    mq->curr_sz = mqtt_mq_currsz(mq);
}

struct mqtt_queued_message* mqtt_mq_find(struct mqtt_message_queue *mq, enum MQTTControlPacketType control_type, uint16_t *packet_id)
{
    struct mqtt_queued_message *curr;
    for(curr = mqtt_mq_get(mq, 0); curr >= mq->queue_tail; --curr) {
        if (curr->control_type == control_type) {
            if ((packet_id == NULL && curr->state != MQTT_QUEUED_COMPLETE) ||
                (packet_id != NULL && *packet_id == curr->packet_id)) {
                return curr;
            }
        }
    }
    return NULL;
}

ssize_t mqtt_unpack_response(struct mqtt_response* response, const uint8_t *buf, size_t bufsz) {
    const uint8_t *const start = buf;
    ssize_t rv = mqtt_unpack_fixed_header(response, buf, bufsz);
    if (rv <= 0) return rv;
    else buf += rv;
    switch(response->fixed_header.control_type) {
        case MQTT_CONTROL_CONNACK:
            rv = mqtt_unpack_connack_response(response, buf);
            break;
        case MQTT_CONTROL_PUBLISH:
            rv = mqtt_unpack_publish_response(response, buf);
            break;
        case MQTT_CONTROL_PUBACK:
            rv = mqtt_unpack_pubxxx_response(response, buf);
            break;
        case MQTT_CONTROL_PUBREC:
            rv = mqtt_unpack_pubxxx_response(response, buf);
            break;
        case MQTT_CONTROL_PUBREL:
            rv = mqtt_unpack_pubxxx_response(response, buf);
            break;
        case MQTT_CONTROL_PUBCOMP:
            rv = mqtt_unpack_pubxxx_response(response, buf);
            break;
        case MQTT_CONTROL_SUBACK:
            rv = mqtt_unpack_suback_response(response, buf);
            break;
        case MQTT_CONTROL_UNSUBACK:
            rv = mqtt_unpack_unsuback_response(response, buf);
            break;
        case MQTT_CONTROL_PINGRESP:
            return rv;
        default:
            return MQTT_ERROR_RESPONSE_INVALID_CONTROL_TYPE;
    }

    if (rv < 0) return rv;
    buf += rv;
    return buf - start;
}

ssize_t __mqtt_pack_uint16(uint8_t *buf, uint16_t integer)
{
  uint16_t integer_htons = MQTT_PAL_HTONS(integer);
  memcpy(buf, &integer_htons, 2);
  return 2;
}

uint16_t __mqtt_unpack_uint16(const uint8_t *buf)
{
  uint16_t integer_htons;
  memcpy(&integer_htons, buf, 2);
  return MQTT_PAL_NTOHS(integer_htons);
}

ssize_t __mqtt_pack_str(uint8_t *buf, const char* str) {
    uint16_t length = (uint16_t)strlen(str);
    int i = 0;

    buf += __mqtt_pack_uint16(buf, length);

    for(; i < length; ++i) {
        *(buf++) = str[i];
    }

    return length + 2;
}

static const char *MQTT_ERRORS_STR[] = {
    "MQTT_UNKNOWN_ERROR",
    __ALL_MQTT_ERRORS(GENERATE_STRING)
};

const char* mqtt_error_str(enum MQTTErrors error) {
    int offset = error - MQTT_ERROR_UNKNOWN;
    if (offset >= 0) {
        return MQTT_ERRORS_STR[offset];
    } else if (error == 0) {
        return "MQTT_ERROR: Buffer too small.";
    } else if (error > 0) {
        return "MQTT_OK";
    } else {
        return MQTT_ERRORS_STR[0];
    }
}

