#pragma once

#include "../Container/ArrayPtr.h"
#include "../IO/Deserializer.h"
#include "../Core/Mutex.h"
#include "../Container/RefCounted.h"
#include "../Core/Thread.h"

namespace Clockwork
{

/// HTTP connection state
enum HttpRequestState
{
    HTTP_INITIALIZING = 0,
    HTTP_ERROR,
    HTTP_OPEN,
    HTTP_CLOSED
};

/// An HTTP connection with response data stream.
class HttpRequest : public RefCounted, public Deserializer, public Thread
{
public:
    /// Construct with parameters.
    HttpRequest(const String& url, const String& verb, const Vector<String>& headers, const String& postData);
    /// Destruct. Release the connection object.
    ~HttpRequest();

    /// Process the connection in the worker thread until closed.
    virtual void ThreadFunction();

    /// Read response data from the HTTP connection and return number of bytes actually read. While the connection is open, will block while trying to read the specified size. To avoid blocking, only read up to as many bytes as GetAvailableSize() returns.
    virtual unsigned Read(void* dest, unsigned size);
    /// Set position from the beginning of the stream. Not supported.
    virtual unsigned Seek(unsigned position);

    /// Return URL used in the request.
    const String& GetURL() const { return url_; }
    /// Return verb used in the request. Default GET if empty verb specified on construction.
    const String& GetVerb() const { return verb_; }
    /// Return error. Only non-empty in the error state.
    String GetError() const;
    /// Return connection state.
    HttpRequestState GetState() const;
    /// Return amount of bytes in the read buffer.
    unsigned GetAvailableSize() const;
    /// Return whether connection is in the open state.
    bool IsOpen() const { return GetState() == HTTP_OPEN; }

private:
    /// Check for end of the data stream and return available size in buffer. Must only be called when the mutex is held by the main thread.
    unsigned CheckEofAndAvailableSize();

    /// URL.
    String url_;
    /// Verb.
    String verb_;
    /// Error string. Empty if no error.
    String error_;
    /// Headers.
    Vector<String> headers_;
    /// POST data.
    String postData_;
    /// Connection state.
    HttpRequestState state_;
    /// Mutex for synchronizing the worker and the main thread.
    mutable Mutex mutex_;
    /// Read buffer for the worker thread.
    SharedArrayPtr<unsigned char> httpReadBuffer_;
    /// Read buffer for the main thread.
    SharedArrayPtr<unsigned char> readBuffer_;
    /// Read buffer read cursor.
    unsigned readPosition_;
    /// Read buffer write cursor.
    unsigned writePosition_;
};

}
