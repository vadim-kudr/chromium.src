// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_FILEAPI_EXTERNAL_FILE_URL_REQUEST_JOB_H_
#define CHROME_BROWSER_CHROMEOS_FILEAPI_EXTERNAL_FILE_URL_REQUEST_JOB_H_

#include <string>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/chromeos/drive/file_errors.h"
#include "net/base/net_errors.h"
#include "net/http/http_byte_range.h"
#include "net/url_request/url_request_job.h"
#include "storage/browser/blob/file_stream_reader.h"
#include "storage/browser/fileapi/file_system_url.h"

namespace net {
class IOBuffer;
class NetworkDelegate;
class URLRequest;
}  // namespace net

namespace chromeos {

// ExternalFileURLRequestJob is the gateway between network-level drive:...
// requests for drive resources and FileSystem.  It exposes content URLs
// formatted as drive:<drive-file-path>.
// The methods should be run on IO thread.
// TODO(hirono): After removing MHTML support, stop to use the special
// externalfile: scheme and use filesystem: URL directly.  crbug.com/415455
class ExternalFileURLRequestJob : public net::URLRequestJob {
 public:
  // Scope of isolated file system.
  class IsolatedFileSystemScope {
   public:
    explicit IsolatedFileSystemScope(const std::string& file_system_id);
    ~IsolatedFileSystemScope();

   private:
    std::string file_system_id_;
    DISALLOW_COPY_AND_ASSIGN(IsolatedFileSystemScope);
  };

  // Callback to take results from an internal helper defined in
  // drive_url_request_job.cc.
  typedef base::Callback<void(
      net::Error,
      const scoped_refptr<storage::FileSystemContext>& file_system_context,
      scoped_ptr<IsolatedFileSystemScope> isolated_file_system_scope,
      const storage::FileSystemURL& file_system_url,
      const std::string& mime_type)> HelperCallback;

  ExternalFileURLRequestJob(void* profile_id,
                            net::URLRequest* request,
                            net::NetworkDelegate* network_delegate);

  // net::URLRequestJob overrides:
  virtual void SetExtraRequestHeaders(
      const net::HttpRequestHeaders& headers) override;
  virtual void Start() override;
  virtual void Kill() override;
  virtual bool GetMimeType(std::string* mime_type) const override;
  virtual bool IsRedirectResponse(GURL* location,
                                  int* http_status_code) override;
  virtual bool ReadRawData(net::IOBuffer* buf,
                           int buf_size,
                           int* bytes_read) override;

 protected:
  virtual ~ExternalFileURLRequestJob();

 private:
  // Called from an internal helper class defined in drive_url_request_job.cc,
  // which is running on the UI thread.
  void OnHelperResultObtained(
      net::Error error,
      const scoped_refptr<storage::FileSystemContext>& file_system_context,
      scoped_ptr<IsolatedFileSystemScope> isolated_file_system_scope,
      const storage::FileSystemURL& file_system_url,
      const std::string& mime_type);

  // Called from FileSystemBackend::GetRedirectURLForContents.
  void OnRedirectURLObtained(const GURL& redirect_url);

  // Called from DriveURLRequestJob::OnFileInfoObtained.
  void OnFileInfoObtained(base::File::Error result,
                          const base::File::Info& file_info);

  // Called when DriveFileStreamReader::Read is completed.
  void OnReadCompleted(int read_result);

  void* const profile_id_;

  // The range of the file to be returned.
  net::HttpByteRange byte_range_;
  int64 remaining_bytes_;

  scoped_refptr<storage::FileSystemContext> file_system_context_;
  scoped_ptr<IsolatedFileSystemScope> isolated_file_system_scope_;
  storage::FileSystemURL file_system_url_;
  std::string mime_type_;
  scoped_ptr<storage::FileStreamReader> stream_reader_;
  GURL redirect_url_;

  // This should remain the last member so it'll be destroyed first and
  // invalidate its weak pointers before other members are destroyed.
  base::WeakPtrFactory<ExternalFileURLRequestJob> weak_ptr_factory_;
  DISALLOW_COPY_AND_ASSIGN(ExternalFileURLRequestJob);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_FILEAPI_EXTERNAL_FILE_URL_REQUEST_JOB_H_
