#ifndef CONNECTION_METADATA_H
#define CONNECTION_METADATA_H

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/endpoint.hpp>
#include <websocketpp/uri.hpp>

#include <server/ServerTypes.h>

using ConnectionHdl = websocketpp::connection_hdl;
using ConnectionPtr = ServerType::connection_ptr;
using HandlePointerPair = std::pair<ConnectionHdl, ConnectionPtr>;

using UriPtr = std::shared_ptr<websocketpp::uri>;

inline bool operator==(ConnectionHdl a, ConnectionHdl b) { return std::owner_less{}(a, b); }

class ConnectionMetadata {
  public:
    enum Status { Connecting, Connected, Closing, Disconnected };

    std::string statusAsString() const {
        switch (m_status) {
        case Connecting:
            return "Connecting";
        case Connected:
            return "Connected";
        case Closing:
            return "Closing";
        case Disconnected:
            return "Disconnected";
        default:
            assert(false);
        }
    }

    ConnectionMetadata(ConnectionHdl hdl, Status status, UriPtr uri)
        : m_hdl(hdl), m_status(status), m_uri(uri) {}

    void setStatus(Status status) { m_status = status; }
    Status getStatus() const { return m_status; }
    UriPtr const &getUri() const { return m_uri; }

  private:
    ConnectionHdl m_hdl;
    Status m_status;
    UriPtr m_uri;
};

using MetadataPtr = std::shared_ptr<ConnectionMetadata>;
using ConnectionList = std::map<ConnectionHdl, MetadataPtr, std::owner_less<ConnectionHdl>>;

#endif