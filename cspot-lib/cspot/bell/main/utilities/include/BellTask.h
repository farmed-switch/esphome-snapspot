#ifndef BELL_TASK_H
#define BELL_TASK_H

#include <string>

#ifdef ESP_PLATFORM
#include <esp_pthread.h>
#include <esp_task.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#elif _WIN32
#include <winsock2.h>
#else
#include <pthread.h>
#endif

#include <iostream>
#include <string>
#include <exception>
#include <stdexcept>
#ifdef ESP_PLATFORM
#include <esp_rom_sys.h>
#endif

namespace bell {
class Task {
 public:
  std::string TASK;
  int stackSize, core;
  bool runOnPSRAM;
  Task(std::string taskName, int stackSize, int priority, int core,

       bool runOnPSRAM = true) {
    this->TASK = taskName;
    this->stackSize = stackSize;
    this->core = core;
    this->runOnPSRAM = runOnPSRAM;
#ifdef ESP_PLATFORM
    this->xStack = NULL;
    this->priority = CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT + priority;
    if (this->priority <= ESP_TASK_PRIO_MIN)
      this->priority = ESP_TASK_PRIO_MIN + 1;
    if (runOnPSRAM) {

      this->xStack = (StackType_t*)heap_caps_malloc(
          this->stackSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }

#endif
  }
  virtual ~Task() {
#ifdef ESP_PLATFORM
    if (xStack)
      heap_caps_free(xStack);
#endif
  }

  bool startTask() {
#ifdef ESP_PLATFORM
    if (runOnPSRAM) {

      xTaskBuffer = (StaticTask_t*)heap_caps_malloc(
          sizeof(StaticTask_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
      return (xTaskCreateStaticPinnedToCore(
                  taskEntryFuncPSRAM, this->TASK.c_str(), this->stackSize, this,
                  this->priority, xStack, xTaskBuffer, this->core) != NULL);
    } else {

      xStack = (StackType_t*)heap_caps_malloc(
          this->stackSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
      xTaskBuffer = (StaticTask_t*)heap_caps_malloc(
          sizeof(StaticTask_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
      if (!xStack || !xTaskBuffer) {
        esp_rom_printf("BellTask: out of internal RAM for task '%s' (%d bytes)\n",
                       this->TASK.c_str(), this->stackSize);
        return false;
      }
      return (xTaskCreateStaticPinnedToCore(
                  taskEntryFuncPSRAM, this->TASK.c_str(), this->stackSize, this,
                  this->priority, xStack, xTaskBuffer, this->core) != NULL);
    }
#endif
#if _WIN32
    thread = CreateThread(NULL, stackSize,
                          (LPTHREAD_START_ROUTINE)taskEntryFunc, this, 0, NULL);
    return thread != NULL;
#else
    if (!pthread_create(&thread, NULL, taskEntryFunc, this)) {
      pthread_detach(thread);
      return true;
    }
    return false;
#endif
  }

 protected:
  virtual void runTask() = 0;

 private:
#if _WIN32
  HANDLE thread;
#else
  pthread_t thread;
#endif
#ifdef ESP_PLATFORM
  int priority;
  StaticTask_t* xTaskBuffer;
  StackType_t* xStack;

  static void taskEntryFuncPSRAM(void* This) {
    Task* self = (Task*)This;
    try {
      self->runTask();
    } catch (const std::exception& e) {

      esp_rom_printf("[BellTask] Uncaught exception in task '%s': %s\n",
             self->TASK.c_str(), e.what());
    } catch (...) {
      esp_rom_printf("[BellTask] Unknown uncaught exception in task '%s'\n",
             self->TASK.c_str());
    }

    esp_rom_printf("[BellTask] task '%s' exiting\n", self->TASK.c_str());

    vTaskDelete(NULL);
  }
#endif

  static void* taskEntryFunc(void* This) {
    ((Task*)This)->runTask();
    return NULL;
  }
};
}

#endif
