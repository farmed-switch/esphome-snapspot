

#ifndef CIVETSERVER_HEADER_INCLUDED
#define CIVETSERVER_HEADER_INCLUDED
#ifdef __cplusplus

#include "civetweb.h"
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef CIVETWEB_CXX_API
#if defined(_WIN32)
#if defined(CIVETWEB_CXX_DLL_EXPORTS)
#define CIVETWEB_CXX_API __declspec(dllexport)
#elif defined(CIVETWEB_CXX_DLL_IMPORTS)
#define CIVETWEB_CXX_API __declspec(dllimport)
#else
#define CIVETWEB_CXX_API
#endif
#elif __GNUC__ >= 4
#define CIVETWEB_CXX_API __attribute__((visibility("default")))
#else
#define CIVETWEB_CXX_API
#endif
#endif

class CivetServer;

class CIVETWEB_CXX_API CivetException : public std::runtime_error
{
  public:
	CivetException(const std::string &msg) : std::runtime_error(msg)
	{
	}
};

class CIVETWEB_CXX_API CivetHandler
{
  public:

	virtual ~CivetHandler()
	{
	}

	virtual bool handleGet(CivetServer *server, struct mg_connection *conn);

	virtual bool handleGet(CivetServer *server,
	                       struct mg_connection *conn,
	                       int *status_code);

	virtual bool handlePost(CivetServer *server, struct mg_connection *conn);

	virtual bool handlePost(CivetServer *server,
	                        struct mg_connection *conn,
	                        int *status_code);

	virtual bool handleHead(CivetServer *server, struct mg_connection *conn);

	virtual bool handleHead(CivetServer *server,
	                        struct mg_connection *conn,
	                        int *status_code);

	virtual bool handlePut(CivetServer *server, struct mg_connection *conn);

	virtual bool handlePut(CivetServer *server,
	                       struct mg_connection *conn,
	                       int *status_code);

	virtual bool handleDelete(CivetServer *server, struct mg_connection *conn);

	virtual bool handleDelete(CivetServer *server,
	                          struct mg_connection *conn,
	                          int *status_code);

	virtual bool handleOptions(CivetServer *server, struct mg_connection *conn);

	virtual bool handleOptions(CivetServer *server,
	                           struct mg_connection *conn,
	                           int *status_code);

	virtual bool handlePatch(CivetServer *server, struct mg_connection *conn);

	virtual bool handlePatch(CivetServer *server,
	                         struct mg_connection *conn,
	                         int *status_code);
};

class CIVETWEB_CXX_API CivetAuthHandler
{
  public:

	virtual ~CivetAuthHandler()
	{
	}

	virtual bool authorize(CivetServer *server, struct mg_connection *conn) = 0;
};

class CIVETWEB_CXX_API CivetWebSocketHandler
{
  public:

	virtual ~CivetWebSocketHandler()
	{
	}

	virtual bool handleConnection(CivetServer *server,
	                              const struct mg_connection *conn);

	virtual void handleReadyState(CivetServer *server,
	                              struct mg_connection *conn);

	virtual bool handleData(CivetServer *server,
	                        struct mg_connection *conn,
	                        int bits,
	                        char *data,
	                        size_t data_len);

	virtual void handleClose(CivetServer *server,
	                         const struct mg_connection *conn);
};

struct CIVETWEB_CXX_API CivetCallbacks : public mg_callbacks {
	CivetCallbacks();
};

class CIVETWEB_CXX_API CivetServer
{
  public:

	CivetServer(const char **options,
	            const struct CivetCallbacks *callbacks = 0,
	            const void *UserContext = 0);
	CivetServer(const std::vector<std::string> &options,
	            const struct CivetCallbacks *callbacks = 0,
	            const void *UserContext = 0);

	virtual ~CivetServer();

	void close();

	const struct mg_context *
	getContext() const
	{
		return context;
	}

	void addHandler(const std::string &uri, CivetHandler *handler);

	void
	addHandler(const std::string &uri, CivetHandler &handler)
	{
		addHandler(uri, &handler);
	}

	void addWebSocketHandler(const std::string &uri,
	                         CivetWebSocketHandler *handler);

	void
	addWebSocketHandler(const std::string &uri, CivetWebSocketHandler &handler)
	{
		addWebSocketHandler(uri, &handler);
	}

	void removeHandler(const std::string &uri);

	void removeWebSocketHandler(const std::string &uri);

	void addAuthHandler(const std::string &uri, CivetAuthHandler *handler);

	void
	addAuthHandler(const std::string &uri, CivetAuthHandler &handler)
	{
		addAuthHandler(uri, &handler);
	}

	void removeAuthHandler(const std::string &uri);

	std::vector<int> getListeningPorts();

	std::vector<struct mg_server_port> getListeningPortsFull();

	static int getCookie(struct mg_connection *conn,
	                     const std::string &cookieName,
	                     std::string &cookieValue);

	static const char *getHeader(struct mg_connection *conn,
	                             const std::string &headerName);

	static const char *getMethod(struct mg_connection *conn);

	static bool getParam(struct mg_connection *conn,
	                     const char *name,
	                     std::string &dst,
	                     size_t occurrence = 0);

	static bool
	getParam(const std::string &data,
	         const char *name,
	         std::string &dst,
	         size_t occurrence = 0)
	{
		return getParam(data.c_str(), data.length(), name, dst, occurrence);
	}

	static bool getParam(const char *data,
	                     size_t data_len,
	                     const char *name,
	                     std::string &dst,
	                     size_t occurrence = 0);

	static std::string getPostData(struct mg_connection *conn);

	static void
	urlDecode(const std::string &src,
	          std::string &dst,
	          bool is_form_url_encoded = true)
	{
		urlDecode(src.c_str(), src.length(), dst, is_form_url_encoded);
	}

	static void urlDecode(const char *src,
	                      size_t src_len,
	                      std::string &dst,
	                      bool is_form_url_encoded = true);

	static void urlDecode(const char *src,
	                      std::string &dst,
	                      bool is_form_url_encoded = true);

	static void
	urlEncode(const std::string &src, std::string &dst, bool append = false)
	{
		urlEncode(src.c_str(), src.length(), dst, append);
	}

	static void
	urlEncode(const char *src, std::string &dst, bool append = false);

	static void urlEncode(const char *src,
	                      size_t src_len,
	                      std::string &dst,
	                      bool append = false);

	const void *
	getUserContext() const
	{
		return UserContext;
	}

  protected:
	class CivetConnection
	{
	  public:
		std::vector<char> postData;
	};

	struct mg_context *context;
	std::map<const struct mg_connection *, CivetConnection> connections;

	const void *UserContext;

  private:

	static int requestHandler(struct mg_connection *conn, void *cbdata);

	static int webSocketConnectionHandler(const struct mg_connection *conn,
	                                      void *cbdata);
	static void webSocketReadyHandler(struct mg_connection *conn, void *cbdata);
	static int webSocketDataHandler(struct mg_connection *conn,
	                                int bits,
	                                char *data,
	                                size_t data_len,
	                                void *cbdata);
	static void webSocketCloseHandler(const struct mg_connection *conn,
	                                  void *cbdata);

	static int authHandler(struct mg_connection *conn, void *cbdata);

	static void closeHandler(const struct mg_connection *conn);

	void (*userCloseHandler)(const struct mg_connection *conn);
};

#endif
#endif