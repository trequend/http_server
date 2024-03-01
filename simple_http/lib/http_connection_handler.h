// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include <functional>

#include "incoming_message.h"
#include "outgoing_message.h"

namespace simple_http {

typedef std::function<void(IncomingMessage& request, OutgoingMessage& response)>
    HttpConnectionHandler;

}
