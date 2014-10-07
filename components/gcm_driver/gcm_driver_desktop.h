// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_GCM_DRIVER_GCM_DRIVER_DESKTOP_H_
#define COMPONENTS_GCM_DRIVER_GCM_DRIVER_DESKTOP_H_

#include <map>
#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "components/gcm_driver/gcm_channel_status_syncer.h"
#include "components/gcm_driver/gcm_client.h"
#include "components/gcm_driver/gcm_connection_observer.h"
#include "components/gcm_driver/gcm_driver.h"

class PrefService;

namespace base {
class FilePath;
class SequencedTaskRunner;
}

namespace extensions {
class ExtensionGCMAppHandlerTest;
}

namespace net {
class URLRequestContextGetter;
}

namespace gcm {

class GCMAccountMapper;
class GCMAppHandler;
class GCMClientFactory;
class GCMDelayedTaskController;

// GCMDriver implementation for desktop and Chrome OS, using GCMClient.
class GCMDriverDesktop : public GCMDriver {
 public:
  GCMDriverDesktop(
      scoped_ptr<GCMClientFactory> gcm_client_factory,
      const GCMClient::ChromeBuildInfo& chrome_build_info,
      PrefService* prefs,
      const base::FilePath& store_path,
      const scoped_refptr<net::URLRequestContextGetter>& request_context,
      const scoped_refptr<base::SequencedTaskRunner>& ui_thread,
      const scoped_refptr<base::SequencedTaskRunner>& io_thread,
      const scoped_refptr<base::SequencedTaskRunner>& blocking_task_runner);
  virtual ~GCMDriverDesktop();

  // GCMDriver overrides:
  virtual void Shutdown() override;
  virtual void OnSignedIn() override;
  virtual void OnSignedOut() override;
  virtual void Purge() override;
  virtual void AddAppHandler(const std::string& app_id,
                             GCMAppHandler* handler) override;
  virtual void RemoveAppHandler(const std::string& app_id) override;
  virtual void AddConnectionObserver(GCMConnectionObserver* observer) override;
  virtual void RemoveConnectionObserver(
      GCMConnectionObserver* observer) override;
  virtual void Enable() override;
  virtual void Disable() override;
  virtual GCMClient* GetGCMClientForTesting() const override;
  virtual bool IsStarted() const override;
  virtual bool IsConnected() const override;
  virtual void GetGCMStatistics(const GetGCMStatisticsCallback& callback,
                                bool clear_logs) override;
  virtual void SetGCMRecording(const GetGCMStatisticsCallback& callback,
                               bool recording) override;
  virtual void SetAccountTokens(
      const std::vector<GCMClient::AccountTokenInfo>& account_tokens) override;
  virtual void UpdateAccountMapping(
      const AccountMapping& account_mapping) override;
  virtual void RemoveAccountMapping(const std::string& account_id) override;

  // Exposed for testing purpose.
  bool gcm_enabled() const { return gcm_enabled_; }
  GCMChannelStatusSyncer* gcm_channel_status_syncer_for_testing() {
    return gcm_channel_status_syncer_.get();
  }

 protected:
  // GCMDriver implementation:
  virtual GCMClient::Result EnsureStarted() override;
  virtual void RegisterImpl(
      const std::string& app_id,
      const std::vector<std::string>& sender_ids) override;
  virtual void UnregisterImpl(const std::string& app_id) override;
  virtual void SendImpl(const std::string& app_id,
                        const std::string& receiver_id,
                        const GCMClient::OutgoingMessage& message) override;

 private:
  class IOWorker;

  //  Stops the GCM service. It can be restarted by calling EnsureStarted again.
  void Stop();

  // Remove cached data when GCM service is stopped.
  void RemoveCachedData();

  void DoRegister(const std::string& app_id,
                  const std::vector<std::string>& sender_ids);
  void DoUnregister(const std::string& app_id);
  void DoSend(const std::string& app_id,
              const std::string& receiver_id,
              const GCMClient::OutgoingMessage& message);

  // Callbacks posted from IO thread to UI thread.
  void MessageReceived(const std::string& app_id,
                       const GCMClient::IncomingMessage& message);
  void MessagesDeleted(const std::string& app_id);
  void MessageSendError(const std::string& app_id,
                        const GCMClient::SendErrorDetails& send_error_details);
  void SendAcknowledged(const std::string& app_id,
                        const std::string& message_id);
  void GCMClientReady(
      const std::vector<AccountMapping>& account_mappings);
  void OnConnected(const net::IPEndPoint& ip_endpoint);
  void OnDisconnected();

  void GetGCMStatisticsFinished(const GCMClient::GCMStatistics& stats);

  scoped_ptr<GCMChannelStatusSyncer> gcm_channel_status_syncer_;

  // Flag to indicate whether the user is signed in to a GAIA account.
  // TODO(jianli): To be removed when sign-in enforcement is dropped.
  bool signed_in_;

  // Flag to indicate if GCM is started.
  bool gcm_started_;

  // Flag to indicate if GCM is enabled.
  // TODO(jianli): Removed when we switch completely to support all users.
  bool gcm_enabled_;

  // Flag to indicate the last known state of the GCM client. Because this
  // flag lives on the UI thread, while the GCM client lives on the IO thread,
  // it may be out of date while connection changes are happening.
  bool connected_;

  // List of observers to notify when connection state changes.
  // Makes sure list is empty on destruction.
  ObserverList<GCMConnectionObserver, true> connection_observer_list_;

  // Account mapper. Only works when user is signed in.
  scoped_ptr<GCMAccountMapper> account_mapper_;

  scoped_refptr<base::SequencedTaskRunner> ui_thread_;
  scoped_refptr<base::SequencedTaskRunner> io_thread_;

  scoped_ptr<GCMDelayedTaskController> delayed_task_controller_;

  // For all the work occurring on the IO thread. Must be destroyed on the IO
  // thread.
  scoped_ptr<IOWorker> io_worker_;

  // Callback for GetGCMStatistics.
  GetGCMStatisticsCallback request_gcm_statistics_callback_;

  // Used to pass a weak pointer to the IO worker.
  base::WeakPtrFactory<GCMDriverDesktop> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(GCMDriverDesktop);
};

}  // namespace gcm

#endif  // COMPONENTS_GCM_DRIVER_GCM_DRIVER_DESKTOP_H_
