/*
    Copyright (c) 2007-2015 Contributors as noted in the AUTHORS file

    This file is part of libzmq, the ZeroMQ core engine in C++.

    libzmq is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    As a special exception, the Contributors give you permission to link
    this library with independent modules to produce an executable,
    regardless of the license terms of these independent modules, and to
    copy and distribute the resulting executable under terms of your choice,
    provided that you also meet, for each linked independent module, the
    terms and conditions of the license of that module. An independent
    module is a module which is not derived from or based on this library.
    If you modify this library, you must extend this exception to your
    version of the library.

    libzmq is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
    License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ZMQ_CURVE_SERVER_HPP_INCLUDED__
#define __ZMQ_CURVE_SERVER_HPP_INCLUDED__

#ifdef ZMQ_HAVE_CURVE

#include "platform.hpp"

#if defined (ZMQ_USE_TWEETNACL)
#   error #include "tweetnacl_base.h"
#   error #include "randombytes.h"
#elif defined (ZMQ_USE_LIBSODIUM)
#   error #include "sodium.h"
#endif

#if crypto_box_NONCEBYTES != 24 \
||  crypto_box_PUBLICKEYBYTES != 32 \
||  crypto_box_SECRETKEYBYTES != 32 \
||  crypto_box_ZEROBYTES != 32 \
||  crypto_box_BOXZEROBYTES != 16 \
||  crypto_secretbox_NONCEBYTES != 24 \
||  crypto_secretbox_ZEROBYTES != 32 \
||  crypto_secretbox_BOXZEROBYTES != 16
#   error "CURVE library not built properly"
#endif

#include "mechanism.hpp"
#include "options.hpp"

namespace zmq
{

    class msg_t;
    class session_base_t;

    class curve_server_t : public mechanism_t
    {
    public:

        curve_server_t (session_base_t *session_,
                        const std::string &peer_address_,
                        const options_t &options_);
        virtual ~curve_server_t ();

        // mechanism implementation
        virtual int next_handshake_command (msg_t *msg_);
        virtual int process_handshake_command (msg_t *msg_);
        virtual int encode (msg_t *msg_);
        virtual int decode (msg_t *msg_);
        virtual int zap_msg_available ();
        virtual status_t status () const;

    private:

        enum state_t {
            expect_hello,
            send_welcome,
            expect_initiate,
            expect_zap_reply,
            send_ready,
            send_error,
            error_sent,
            connected
        };

        session_base_t * const session;

        const std::string peer_address;

        //  Current FSM state
        state_t state;

        //  Status code as received from ZAP handler
        std::string status_code;

        uint64_t cn_nonce;
        uint64_t cn_peer_nonce;

        //  Our secret key (s)
        uint8_t secret_key [crypto_box_SECRETKEYBYTES];

        //  Our short-term public key (S')
        uint8_t cn_public [crypto_box_PUBLICKEYBYTES];

        //  Our short-term secret key (s')
        uint8_t cn_secret [crypto_box_SECRETKEYBYTES];

        //  Client's short-term public key (C')
        uint8_t cn_client [crypto_box_PUBLICKEYBYTES];

        //  Key used to produce cookie
        uint8_t cookie_key [crypto_secretbox_KEYBYTES];

        //  Intermediary buffer used to speed up boxing and unboxing.
        uint8_t cn_precom [crypto_box_BEFORENMBYTES];

        int process_hello (msg_t *msg_);
        int produce_welcome (msg_t *msg_);
        int process_initiate (msg_t *msg_);
        int produce_ready (msg_t *msg_);
        int produce_error (msg_t *msg_) const;

        void send_zap_request (const uint8_t *key);
        int receive_and_process_zap_reply ();
        mutex_t sync;
    };

}

#endif

#endif
