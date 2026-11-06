# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/11/05 13:47:52 by ezararsi          #+#    #+#              #
#    Updated: 2026/11/05 21:56:30 by ezararsi         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME    = minishell
CC      = cc
CFLAGS  = -Wall -Wextra -Werror -g -Iinclude
LIBS    = -lreadline

# macOS: Apple liefert nur libedit statt GNU readline -> Homebrew-Version nutzen
#        (einmalig: brew install readline)
ifeq ($(shell uname),Darwin)
    RL_DIR  = $(shell brew --prefix readline)
    CFLAGS += -I$(RL_DIR)/include
    LIBS    = -L$(RL_DIR)/lib -lreadline
endif

SRCS    = src/main.c src/input.c src/lexer.c src/parser.c src/expand.c src/heredoc.c \
          src/exec.c src/redirect.c src/path.c src/builtins.c \
          src/builtins2.c src/env.c src/signals.c src/utils.c
OBJS    = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LIBS)

%.o: %.c include/minishell.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

test: all
	@bash tests/run_tests.sh

.PHONY: all clean fclean re test
