#pragma once

#ifdef _WIN32
#pragma warning(disable:4996)
#define NOMINMAX
#include <winsock2.h>
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <mmsystem.h>
#include <ppl.h>
#endif

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <codecvt>
#include <locale>
#include <sstream>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include <functional>
#include <future>
#include <mutex>
#include <memory>

#define POCO_STATIC
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/DirectoryIterator.h"
#include "Poco/Timestamp.h"
#include "Poco/URI.h"
#include "Poco/StreamCopier.h"
#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/PartHandler.h"
#include "Poco/Net/PartSource.h"
#include "Poco/Net/FilePartSource.h"
#include "Poco/Net/MessageHeader.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/SocketAddress.h"
