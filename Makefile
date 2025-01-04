# configuration
CC = gcc
CFLAGS = -std=c89 -Wall

SRC_DIR = src
SOURCES = $(wildcard $(SRC_DIR)/*.c)

OBJ_DIR = obj
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

TARGET = rem8C


.PHONY : all clean test

# building
all: $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) -o $@ $^

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ 

$(OBJ_DIR) :
	@mkdir -p $(OBJ_DIR)

# cleaning
clean:
	rm -v -f $(OBJECTS)
	rmdir $(OBJ_DIR)
	rm $(TARGET)