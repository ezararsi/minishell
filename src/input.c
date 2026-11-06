/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   input.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/06 13:20:11 by ezararsi          #+#    #+#             */
/*   Updated: 2026/11/06 13:20:11 by ezararsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Ohne Terminal (z.B. echo "ls" | ./minishell) wird kein Prompt
** ausgegeben, wie bei bash. Gelesen wird Byte fuer Byte, damit nichts
** nach dem '\n' verbraucht wird - der Rest gehoert evtl. einem
** Programm oder Heredoc, das danach von stdin liest.
*/
static char	*read_plain_line(void)
{
	char	*line;
	char	c;
	ssize_t	n;

	line = safe_strdup("");
	n = read(STDIN_FILENO, &c, 1);
	while (n > 0 && c != '\n')
	{
		line = join_n(line, &c, 1);
		n = read(STDIN_FILENO, &c, 1);
	}
	if (n <= 0 && *line == '\0')
	{
		free(line);
		return (NULL);
	}
	return (line);
}

bool	is_interactive(void)
{
	return (isatty(STDIN_FILENO));
}

char	*read_input(const char *prompt)
{
	if (is_interactive())
		return (readline(prompt));
	return (read_plain_line());
}
