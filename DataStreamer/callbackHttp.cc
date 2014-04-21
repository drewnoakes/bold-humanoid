#include "datastreamer.ih"

int DataStreamer::callback_http(
  libwebsocket_context* context,
  libwebsocket* wsi,
  libwebsocket_callback_reasons reason,
  void* /*user*/,
  void* in,
  size_t /*len*/)
{
  switch (reason)
  {
  case LWS_CALLBACK_HTTP:
  {
    ASSERT(ThreadUtil::isDataStreamerThread());

    // TODO: make this std::string, or copy from string constant correctly
    char* path;
    if (in)
    {
      path = (char*)in;
      if (strcmp(path, "/") == 0)
      {
        path = (char*)"/index.html";
      }
      else if (string(path).find("..") != string::npos)
      {
        // protect against ../ attacks
        log::info("DataStreamer::callback_http") << "invalid request path: " << path;
        return 1;
      }

      // ignore query string (for now)
      size_t queryStart = string(path).find("?");
      if (queryStart != string::npos)
      {
        path[queryStart] = 0;
      }

      log::info("DataStreamer::callback_http") << "requested path: " << path;
    }
    else
    {
      path = (char*)"/index.html";
    }

    //
    // Determine the MIME type based upon the path extension
    //
    auto extension = string(path).substr(string(path).find_last_of(".") + 1);
    string mimeType = "application/binary";
    if (extension == "html") {
      mimeType = "text/html";
    }
    else if (extension == "js") {
      mimeType = "text/javascript";
    }
    else if (extension == "json") {
      mimeType = "application/json";
    }
    else if (extension == "css") {
      mimeType = "text/css";
    }
    else if (extension == "png") {
      mimeType = "image/png";
    }
    else if (extension == "jpg") {
      mimeType = "image/jpeg";
    }
    else if (extension == "ico") {
      mimeType = "image/x-icon";
    }
    else if (extension == "map") {
      mimeType = "application/json";
    }

    char buf[256];
    sprintf(buf, "www%s", path);

    if (libwebsockets_serve_http_file(context, wsi, buf, mimeType.c_str(), nullptr))
      lwsl_err("Failed to send HTTP file\n");

    break;
  }
  case LWS_CALLBACK_HTTP_FILE_COMPLETION:
  {
    // async sending of file completed. kill the connection.
    ASSERT(ThreadUtil::isDataStreamerThread());
    return 1;
  }
  default:
    // Unhandled reason
    break;
  }

  return 0;
}
