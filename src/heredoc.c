/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 15:40:06 by ezararsi          #+#    #+#             */
/*   Updated: 2026/11/05 21:53:25 by ezararsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Liest den Heredoc in einem Kindprozess. Vorteil: Ctrl-C beendet nur das
** Kind, die Shell merkt es an waitpid und bricht den Befehl sauber ab.
*/
static void	heredoc_child(t_redir *r, int fd, t_shell *sh)
{
	char	*line;
	char	*tmp;

	signals_default();
	while (1)
	{
		line = read_input("> ");
		if (!line)
		{
			print_error("warning", "here-document delimited by end-of-file");
			break ;
		}
		if (strcmp(line, r->file) == 0)
		{
			free(line);
			break ;
		}
		if (r->expand)
		{
			tmp = expand(line, sh, true, false);
			free(line);
			line = tmp;
		}
		write(fd, line, strlen(line));
		write(fd, "\n", 1);
		free(line);
	}
	close(fd);
	exit(0);
}

/*
** Inhalt landet in einer Temp-Datei. Nach dem open() wird sie direkt
** geloescht (unlink) - der fd bleibt trotzdem gueltig.
*/
static int	read_heredoc(t_redir *r, t_shell *sh)
{
	char	path[] = "/tmp/.minishell_heredoc_XXXXXX";
	int		fd;
	int		status;
	pid_t	pid;

	fd = mkstemp(path);
	if (fd < 0)
		return (perror("minishell: heredoc"), -1);
	pid = fork();
	if (pid < 0)
	{
		perror("minishell: fork");
		close(fd);
		unlink(path);
		return (-1);
	}
	if (pid == 0)
		heredoc_child(r, fd, sh);
	close(fd);
	signals_ignore();
	waitpid(pid, &status, 0);
	signals_prompt();
	if (WIFSIGNALED(status))
	{
		write(STDOUT_FILENO, "\n", 1);
		sh->exit_status = 130;
		unlink(path);
		return (-1);
	}
	r->fd = open(path, O_RDONLY);
	unlink(path);
	if (r->fd < 0)
		return (perror("minishell: heredoc"), -1);
	return (0);
}

/* Alle Heredocs werden VOR der Ausfuehrung gelesen */
int	handle_heredocs(t_cmd *cmds, t_shell *sh)
{
	t_redir	*r;

	while (cmds)
	{
		r = cmds->redirs;
		while (r)
		{
			if (r->type == HEREDOC && read_heredoc(r, sh) < 0)
				return (-1);
			r = r->next;
		}
		cmds = cmds->next;
	}
	return (0);
}
