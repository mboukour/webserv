#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "../AHttp/AHttp.hpp"

#include "../../Server/Server.hpp"
#include "../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include "../../Cgi/Cgi.hpp"
#include <cstddef>
#include <string>

class HttpRequest;
class CgiState;
// HttpResponse will check everything
class HttpResponse: public AHttp {
    private:
        const int clientFd;
        const int epollFd;
        int statusCode;
        enum PostState {INIT_POST, NEW_REQ_ENTRY, LAST_ENTRY};
        enum ChunkState {CH_START, CH_SIZE, CH_ERROR, CH_DATA, CH_COMPLETE, CH_TRAILER};
        enum MultiState {M_HEADERS, M_BOUND, M_BODY};
        PostState postState;
        int fd;
        std::string fileName;
        ChunkState chunkState;
        size_t remaining_chunk_size;
        size_t offset;
        std::string chunkBody;
        size_t left;
        std::string success_create;
        std::string success_delete;
        std::string reasonPhrase;
        std::vector<std::string> cookies;
        std::string packet;
        std::string prev_chunk_size;
        bool pendingCRLF;
        bool isLastEntry;
        MultiState multiState;
        size_t currBound;
        std::string subHeaders;
        std::string multiBody;
        bool skip;
        bool hasWritten;
        CgiState *cgiState;
        bool isChunked;

        static std::string getConTypeExten(const std::string &contentType);
        static std::string extToNature(const std::string &extension);
        static std::pair<std::string, std::string> newMapNode(const HttpRequest &request, std::string const &str);
        static bool isCgiFile(const std::string &filePath, const HttpRequest &request);
        static bool isReturnRequest(const HttpRequest &request);
        static std::map<std::string, std::string> strToHeaderMap(const HttpRequest &request, std::string &str);
        static std::map<std::string, std::string> extractFileInfo(const HttpRequest &request, std::string const &str);
        void handleAutoIndex(const HttpRequest& request);
        // std::string makeCgiResponse(const HttpRequest &request);
        bool handleReturnDirective(const HttpRequest &request) const;
        bool handleSessionTest(const HttpRequest &request) const;
        void sendResponse(void) const;
        void handleDeleteRequest(const HttpRequest &request);
        void handleGetRequest(const HttpRequest &request);
        void handlePostRequest(const HttpRequest &request);
        void postResponse(const HttpRequest& request,
            int statusCode, std::string body, std::string const fileName);
        void chunkedTransfer(const HttpRequest &request);
        void setPacket(const HttpRequest &request);
        void multiForm(const HttpRequest &request);
        void multiChunked(const HttpRequest &request);
        void multiForm_chunked(const HttpRequest &request);
        std::string generateFileName(const HttpRequest &request, std::string &file);
        std::string getFileLastModifiedTime(const std::string &filePath);

    public:
        HttpResponse();
        ~HttpResponse();
        HttpResponse(const HttpRequest &request, int clientFd, int epollFd);
        HttpResponse(const std::string &version, int statusCode, const std::string &reasonPhrase, const std::string &body);
        void addNeededHeaders(void);
        void addCookie(const std::string& name, const std::string& value, const std::string& attributes);
        void setBody(const std::string &body);
        bool removeDirectory(const std::string &path);
        void handleNewReqEntry(const HttpRequest &request);
        bool getIsLastEntry(void) const;
        std::string toString(void) const;
        int getFd(void) const;
        bool hasPostTimedOut(void) const;
        // void multiForm(const HttpRequest &request);
};

#endif
