/*
  Client.h - Base class that provides Client
  Copyright (c) 2011 Adrian McEwen.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef client_h
#define client_h
#include <stdint.h>
#include <stdio.h>

typedef uint8_t IPAddress_t[4];

typedef int     (*fpClient_connectIP)   (IPAddress_t ip, uint16_t port);
typedef int     (*fpClient_connectHost) (const char *host, uint16_t port);
typedef uint8_t (*fpClient_connected)   (void);
typedef size_t  (*fpClient_write)       (uint8_t);
typedef size_t  (*fpClient_writeMulti)  (const uint8_t *buf, size_t size);
typedef int     (*fpClient_available)   (void);
typedef int     (*fpClient_read)        (void);
typedef int     (*fpClient_readMulti)   (uint8_t *buf, size_t size);
typedef int     (*fpClient_peek)        (void);
typedef void    (*fpClient_flush)       (void);
typedef void    (*fpClient_stop)        (void);

typedef struct
{
    fpClient_connectIP      connectIP;
    fpClient_connectHost    connectHost;
    fpClient_connected      connected;
    fpClient_write          write;
    fpClient_writeMulti     writeMulti;
    fpClient_available      available;
    fpClient_read           read;
    fpClient_readMulti      readMulti;
    fpClient_peek           peek;
    fpClient_flush          flush;
    fpClient_stop           stop;
} Client_t;

#endif
