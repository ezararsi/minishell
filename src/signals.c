/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 15:07:08 by ezararsi          #+#    #+#             */
/*   Updated: 2026/11/05 16:32:17 by ezararsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

volatile sig_atomic_t	g_signal = 0;

/*
** Ctrl-C am Prompt: Eingabe verwerfen und neuen Prompt anzeigen.
** Die Shell selbst wird dabei NICHT beendet.
*/
static void	handle_sigint(int sig)
{
	g_signal = sig;
	write(STDOUT_FILENO, "\n", 1);
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
}

static void	set_signal(int sig, void (*handler)(int))
{
	struct sigaction	sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sigaction(sig, &sa, NULL);
}

/* Shell wartet auf Eingabe */
void	signals_prompt(void)
{
	set_signal(SIGINT, handle_sigint);
	set_signal(SIGQUIT, SIG_IGN);
}

/* Shell wartet auf Kindprozesse: Signale sollen nur das Kind treffen */
void	signals_ignore(void)
{
	set_signal(SIGINT, SIG_IGN);
	set_signal(SIGQUIT, SIG_IGN);
}

/* Im Kindprozess vor execve: normales Verhalten wiederherstellen */
void	signals_default(void)
{
	set_signal(SIGINT, SIG_DFL);
	set_signal(SIGQUIT, SIG_DFL);
	set_signal(SIGPIPE, SIG_DFL);
}
