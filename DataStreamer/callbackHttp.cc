#include "datastreamer.ih"

int DataStreamer::callback_http(
  struct libwebsocket_context* context,
  struct libwebsocket* wsi,
  enum libwebsocket_callback_reasons reason,
  void* /*user*/,
  void* in,
  size_t /*len*/)
{

  switch (reason)
  {
    case LWS_CALLBACK_HTTP:
    {
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
          std::cout << "[DataStreamer::callback_http] invalid request path: " << path << std::endl;
          return 1;
        }

        // ignore query string (for now)
        int queryStart = string(path).find("?");
        if (queryStart != string::npos)
        {
          path[queryStart] = 0;
        }

        std::cout << "[DataStreamer::callback_http] requested path: " << path << std::endl;
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

      char buf[256];
      sprintf(buf, "www%s", path);

      if (libwebsockets_serve_http_file(context, wsi, buf, mimeType.c_str()))
        lwsl_err("Failed to send HTTP file\n");

      break;
    }
    case LWS_CALLBACK_HTTP_FILE_COMPLETION:
    {
      // async sending of file completed. kill the connection.
      return 1;
    }
  }

  return 0;
}
