# ----------------------------
# Makefile Options
# ----------------------------

NAME = CINEMA
ICON = icon.png
DESCRIPTION = "USB Video Player -- William Wierzbowski"
COMPRESSED = YES

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
