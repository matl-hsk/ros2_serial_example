/****************************************************************************
 *
 * Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>

#include <poll.h>

class Transport_node
{
public:
    Transport_node();
    virtual ~Transport_node();

    virtual int init() {return 0;}
    virtual uint8_t close() {return 0;}
    ssize_t read(uint8_t *topic_ID, uint8_t out_buffer[], size_t buffer_len);

    /**
     * write a buffer
     * @param topic_ID
     * @param buffer buffer to write: it must leave get_header_length() bytes free at the beginning. This will be
     *               filled with the header. length does not include get_header_length(). So buffer looks like this:
     *                -------------------------------------------------
     *               | header (leave free)          | payload data     |
     *               | get_header_length() bytes    | length bytes     |
     *                -------------------------------------------------
     * @param length buffer length excluding header length
     * @return length on success, <0 on error
     */
    ssize_t write(const uint8_t topic_ID, uint8_t buffer[], size_t length);

    /** Get the Length of struct Header to make headroom for the size of struct Header along with payload */
    ssize_t get_header_length();

protected:
    virtual ssize_t node_read(void *buffer, size_t len) = 0;
    virtual ssize_t node_write(void *buffer, size_t len) = 0;
    virtual bool fds_OK() = 0;
    uint16_t crc16_byte(uint16_t crc, const uint8_t data);
    uint16_t crc16(uint8_t const *buffer, size_t len);

    uint32_t rx_buff_pos;
    uint8_t rx_buffer[1024] = {};

private:
    struct __attribute__((packed)) Header {
        uint8_t marker[3];
        uint8_t topic_ID;
        uint8_t seq;
        uint8_t payload_len_h;
        uint8_t payload_len_l;
        uint8_t crc_h;
        uint8_t crc_l;
    };
};

class UART_node: public Transport_node
{
public:
    UART_node(const char *uart_name, uint32_t baudrate, uint32_t poll_ms);
    virtual ~UART_node();

    int init() override;
    uint8_t close() override;

protected:
    ssize_t node_read(void *buffer, size_t len);
    ssize_t node_write(void *buffer, size_t len);
    bool fds_OK();

    int uart_fd;
    char uart_name[64] = {};
    uint32_t baudrate;
    uint32_t poll_ms;
    struct pollfd poll_fd[1] = {};
};
