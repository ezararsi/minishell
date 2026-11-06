/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 18:15:18 by ezararsi          #+#    #+#             */
/*   Updated: 2026/11/05 19:37:48 by ezararsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Ablauf fuer jede Eingabezeile:
**   1. lexer    "ls -l | wc"  ->  Tokens
**   2. parser   Tokens        ->  Liste von Befehlen (+ $VAR, Quotes)
**   3. heredocs vorab einlesen
**   4. execute  fork / pipe / execve
*/
static void	process_line(char *line, t_shell *sh)
{
	t_token	*tokens;
	t_cmd	*cmds;
	bool	error;

	tokens = lexer(line, &error);
	if (error)
	{
		sh->exit_status = 2;
		return ;
	}
	if (!tokens)
		return ;
	cmds = parser(tokens, sh);
	free_tokens(tokens);
	if (!cmds)
	{
		sh->exit_status = 2;
		return ;
	}
	if (handle_heredocs(cmds, sh) == 0)
		execute(cmds, sh);
	free_cmds(cmds);
}

static void	init_shell(t_shell *sh, char **envp)
{
	sh->env = env_init(envp);
	sh->exit_status = 0;
	sh->should_exit = false;
	sh->is_child = false;
	rl_catch_signals = 0;
}

int	main(int argc, char **argv, char **envp)
{
	t_shell	sh;
	char	*line;

	(void)argc;
	(void)argv;
	init_shell(&sh, envp);
	while (!sh.should_exit)
	{
		signals_prompt();
		line = read_input("minishell$ ");
		if (g_signal == SIGINT)
		{
			sh.exit_status = 130;
			g_signal = 0;
		}
		if (!line)
			break ;
		if (*line && is_interactive())
			add_history(line);
		process_line(line, &sh);
		free(line);
	}
	if (!sh.should_exit && is_interactive())
		printf("exit\n");
	rl_clear_history();
	env_free(sh.env);
	return (sh.exit_status);
}
