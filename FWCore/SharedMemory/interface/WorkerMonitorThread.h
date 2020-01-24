#ifndef FWCore_SharedMemory_WorkerMonitorThread_h
#define FWCore_SharedMemory_WorkerMonitorThread_h
// -*- C++ -*-
//
// Package:     FWCore/SharedMemory
// Class  :     WorkerMonitorThread
//
/**\class WorkerMonitorThread WorkerMonitorThread.h " FWCore/SharedMemory/interface/WorkerMonitorThread.h"

 Description: Manages a thread that monitors the worker process for unix signals

 Usage:
      Allows a user settable action to happen in the case of a unix signal occuring.
*/
//
// Original Author:  Chris Jones
//         Created:  21/01/2020
//

// system include files
#include <atomic>
#include <functional>
#include <thread>
#include <signal.h>

// user include files
#include "FWCore/Utilities/interface/thread_safety_macros.h"

// forward declarations

namespace edm::shared_memory {
  class WorkerMonitorThread {
  public:
    WorkerMonitorThread() {}
    ~WorkerMonitorThread() { stopRequested_.store(true); }
    WorkerMonitorThread(const WorkerMonitorThread&) = delete;
    const WorkerMonitorThread& operator=(const WorkerMonitorThread&) = delete;

    // ---------- const member functions ---------------------

    // ---------- member functions ---------------------------
    void setAction(std::function<void()> iFunc) {
      action_ = std::move(iFunc);
      actionSet_.store(true);
    }

    void startThread();

    ///Sets the unix signal handler which communicates with the thread.
    void setupSignalHandling();

    void stop() { stopRequested_ = true; }

  private:
    static void sig_handler(int sig, siginfo_t*, void*);
    void run();
    // ---------- member data --------------------------------
    std::atomic<bool> stopRequested_ = false;
    std::atomic<bool> helperReady_ = false;
    std::atomic<bool> actionSet_ = false;
    CMS_THREAD_GUARD(actionSet_) std::function<void()> action_;

    static std::atomic<bool> s_signal_happened;
    static std::atomic<bool> s_helperThreadDone;
    CMS_THREAD_GUARD(s_signal_happened) static int s_sig;

    std::thread helperThread_;
  };
}  // namespace edm::shared_memory

#endif
