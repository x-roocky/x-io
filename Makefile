.PHONY: all run clean

NAME = xio_test
OBJS = main.o \
       xio/xio-arch.o \
       xio/xio.o

%.o: %.c
	@cc -c $< -o $@

$(NAME): $(OBJS)
	@cc $^ -o $(NAME)

all: $(NAME)

run: $(NAME)
	./$^

clean:
	rm -rf $(OBJS) $(NAME)
