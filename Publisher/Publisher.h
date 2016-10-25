#pragma once

void publishing_loop(SOCKET* socket);
bool publish(ByteArray message, ByteArray topic, SOCKET* socket);
ByteArray make_package(ByteArray message, ByteArray topic);