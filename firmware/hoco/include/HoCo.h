#pragma once

typedef void (*subscribe_callback)(char *subtopic);
typedef void (*publish_callback)(char *subtopic, char *data, bool retain);
