/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirect.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 15:38:02 by ezararsi          #+#    #+#             */
/*   Updated: 2026/11/05 16:51:36 by ezararsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Leitet stdin/stdout auf Dateien um.
**   <   Datei lesen         ->  fd 0
**   >   Datei ueberschreiben ->  fd 1
**   >>  an Datei anhaengen   ->  fd 1
**   <<  Heredoc (Temp-Datei) ->  fd 0
*/
int	apply_redirs(t_redir *r)
{
	int	fd;

	while (r)
	{
		if (r->type == HEREDOC)
		{
			fd = r->fd;
			r->fd = -1;
		}
		else if (r->type == REDIR_IN)
			fd = open(r->file, O_RDONLY);
		else if (r->type == REDIR_OUT)
			fd = open(r->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		else
			fd = open(r->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
		if (fd < 0)
		{
			print_error(r->file, strerror(errno));
			return (-1);
		}
		if (r->type == REDIR_IN || r->type == HEREDOC)
			dup2(fd, STDIN_FILENO);
		else
			dup2(fd, STDOUT_FILENO);
		close(fd);
		r = r->next;
	}
	return (0);
}
