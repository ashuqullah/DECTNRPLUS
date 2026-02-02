#pragma once
#include <stdint.h>

int fping_server_start(void);
int fping_client_start(uint32_t count);
void fping_stop(void);
enum fping_pkt_type {
    FPING_PKT_BEGIN = 1,
    FPING_PKT_CHUNK = 2,
    FPING_PKT_END   = 3,
};

struct fping_begin {
    uint8_t  type;
    uint32_t total_len;
    uint32_t crc32;
};

struct fping_chunk {
    uint8_t  type;
    uint16_t seq;
    uint16_t len;
    uint8_t  data[0];
};

struct fping_end {
    uint8_t  type;
};
