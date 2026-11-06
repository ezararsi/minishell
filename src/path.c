/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   path.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 15:04:27 by ezararsi          #+#    #+#             */
/*   Updated: 2026/11/05 22:36:38 by ezararsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Sucht ein Programm in $PATH.
** "ls" -> probiert /usr/local/bin/ls, /usr/bin/ls, /bin/ls ...
** Enthaelt der Name ein '/', wird er direkt verwendet (./a.out, /bin/ls).
*/
char	*find_path(char *cmd, t_shell *sh)
{
	char	*path_env;
	char	*paths;
	char	*dir;
	char	*full;
	size_t	len;

	if (strchr(cmd, '/'))
		return (safe_strdup(cmd));
	path_env = env_get(sh->env, "PATH");
	if (!path_env || !*cmd)
		return (NULL);
	paths = safe_strdup(path_env);
	dir = strtok(paths, ":");
	while (dir)
	{
		len = strlen(dir) + strlen(cmd) + 2;
		full = safe_malloc(len);
		snprintf(full, len, "%s/%s", dir, cmd);
		if (access(full, X_OK) == 0)
		{
			free(paths);
			return (full);
		}
		free(full);
		dir = strtok(NULL, ":");
	}
	free(paths);
	return (NULL);
}
