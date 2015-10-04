#define _LARGEFILE64_SOURCE

#include <cstdbool>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cassert>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "bitcoinconsensus.h"

static const uint8_t txToBegin[] = "\x01\x00\x00\x00\x01\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00";
static const uint8_t txToEnd[] = "\xff\xff\xff\xff\x01\x00\x00\x00\x00\x00\x00\x00\x00\x01\x87\x00\x00\x00\x00";

int main(int argc, char *argv[]) {
    int fd;
    uint8_t buffer[4 + 4 + 4 + 4 + 1000000 + 1000000];
    off64_t file_length;
    
    uint8_t scriptPubKey[1000000], txTo[1000000];
    uint32_t scriptPubKeyLen, txToScriptLen, txToLen, nIn, flags;
    
    bitcoinconsensus_error err = (bitcoinconsensus_error)0xff;
    
    if (argc < 2) {
        printf("argc < 2\n");
        abort();
    }
    
    errno = 0;
    
    fd = open(argv[1], O_RDONLY|O_NOATIME);

    if (fd < 0) {
        perror("fopen");
        abort();
    }
    
    file_length = lseek64(fd, 0, SEEK_END);
    if (file_length < 0) {
        perror("lseek64 SEEK_END");
        abort();
    }
    
    if (file_length > (off64_t)sizeof(buffer)) {
        printf("file_length %ld > sizeof(buffer) %lu\n", file_length, sizeof(buffer));
        return 1;
    }
    
    if (lseek64(fd, 0, SEEK_SET) != 0) {
        perror("lseek64 SEEK_SET 0");
        abort();
    }
    
    if (file_length < 0) {
        perror("lseek");
        abort();
    }
    
    ssize_t read_ret = read(fd, buffer, file_length);
    
    if (read_ret < 0) {
        perror("read");
        abort();
    }
    
    if (read_ret != file_length) {
        printf("read_ret %ld != file_length %ld\n", read_ret, file_length);
        abort();
    }
    
    if (file_length < 1) {
        printf("file_length < 1\n");
        return 1;
    }
    
    // restrict scriptPubKey to 256 bytes actual limit is much higher
    scriptPubKeyLen = buffer[0];
    
    if (scriptPubKeyLen > file_length - 1) {
        printf("scriptPubKeyLen > file_length - 1\n");
        return 1;
    }
    
    txToScriptLen = file_length - scriptPubKeyLen - 1;
    txToLen = sizeof(txToBegin)-1 + txToScriptLen + sizeof(txToEnd)-1;
    nIn = 0;
    flags = 0;
    
    if (scriptPubKeyLen > 1000000) {
        printf("scriptPubKeyLen > 1000000\n");
        return 1;
    }
    
    if (txToLen > 1000000) {
        printf("scriptPubKeyLen > 1000000\n");
        return 1;
    }
    
    memcpy(scriptPubKey, buffer + 1, scriptPubKeyLen);
    
    //build txTo
    off_t offset = 0;
    memcpy(txTo + offset, txToBegin, sizeof(txToBegin)-1);
    offset += sizeof(txToBegin)-1;

    // build var_int for txToScriptLen
    if (txToScriptLen < 0xfd) {
        uint8_t t = txToScriptLen;
        memcpy(txTo + offset, &t, sizeof(t));
        offset += sizeof(t);
        txToLen += sizeof(t);
    }
    else if (txToScriptLen < 0xffff) {
        uint16_t t = txToScriptLen;
        txTo[offset++] = 0xfd;
        memcpy(txTo + offset, &t, sizeof(t));
        offset += sizeof(t);
        txToLen += 1 + sizeof(t);
    }
    else if (txToScriptLen < 0xffffffff) {
        uint32_t t = txToScriptLen;
        txTo[offset++] = 0xfe;
        memcpy(txTo + offset, &t, sizeof(t));
        offset += sizeof(t);
        txToLen += 1 + sizeof(t);
    }
    else {
        uint64_t t = txToScriptLen;
        txTo[offset++] = 0xff;
        memcpy(txTo + offset, &t, sizeof(t));
        offset += sizeof(t);
        txToLen += 1 + sizeof(t);
    }
    memcpy(txTo + offset, buffer + 1 + scriptPubKeyLen, txToScriptLen);
    offset += txToScriptLen;
    memcpy(txTo + offset, txToEnd, sizeof(txToEnd)-1);
    offset += sizeof(txToEnd)-1;
    
    assert(offset == txToLen);
    
    if (argc > 2) {
        printf("%u %u %u %u %u ", scriptPubKeyLen, txToScriptLen, txToLen, nIn, flags);
        
        for (uint32_t i=0; i<scriptPubKeyLen; ++i) {
            printf("%02X", scriptPubKey[i]);
        }
        printf(" ");
        
        for (uint32_t i=0; i<txToLen; ++i) {
            printf("%02X", txTo[i]);
        }
        printf("\n");
        
        if (argc > 3)
            return 0;
    }
    
    int result;

#if BITCOINCONSENSUS_API_VER == 0
    result = bitcoinconsensus_verify_script(scriptPubKey, scriptPubKeyLen, txTo, txToLen, nIn, flags, &err);
#elif BITCOINCONSENSUS_API_VER == 1
    result = bitcoinconsensus_verify_script(scriptPubKey, scriptPubKeyLen, txTo, txToLen, 0, 0, 0, 0, nIn, flags, &err);
#else
    #error Invalid BITCOINCONSENSUS_API_VER
#endif
    
    if (result != 1) {
        switch (err) {
            case bitcoinconsensus_ERR_OK:
                printf("bitcoinconsensus_ERR_OK\n");
                break;
            case bitcoinconsensus_ERR_TX_INDEX:
                printf("bitcoinconsensus_ERR_TX_INDEX\n");
                break;
            case bitcoinconsensus_ERR_TX_SIZE_MISMATCH:
                printf("bitcoinconsensus_ERR_TX_INDEX\n");
                break;
            case bitcoinconsensus_ERR_TX_DESERIALIZE:
                printf("bitcoinconsensus_ERR_TX_INDEX\n");
                break;
            default:
                printf("invalid bitcoinconsensus_error\n");
                break;
        }
        
        if (result > 0) {
            return result;
        }
        return 1;
    }
    
    return 0;
}
