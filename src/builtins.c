/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 16:07:50 by ezararsi          #+#    #+#             */
/*   Updated: 2026/11/05 22:07:32 by ezararsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

bool	is_builtin(char *name)
{
	return (strcmp(name, "echo") == 0 || strcmp(name, "cd") == 0
		|| strcmp(name, "pwd") == 0 || strcmp(name, "export") == 0
		|| strcmp(name, "unset") == 0 || strcmp(name, "env") == 0
		|| strcmp(name, "exit") == 0);
}

/*
** Gueltig sind "-n", "-nn", "-nnn" ... (wie in bash).
** Nicht gueltig: "-", "-nx", "--n" -> werden normal ausgegeben.
*/
static bool	is_n_flag(char *s)
{
	int	i;

	if (s[0] != '-' || s[1] != 'n')
		return (false);
	i = 1;
	while (s[i] == 'n')
		i++;
	return (s[i] == '\0');
}

/* echo [-n] text...   (-n = keine neue Zeile am Ende) */
static int	builtin_echo(char **argv)
{
	int		i;
	bool	newline;

	i = 1;
	newline = true;
	while (argv[i] && is_n_flag(argv[i]))
	{
		newline = false;
		i++;
	}
	while (argv[i])
	{
		printf("%s", argv[i]);
		if (argv[i + 1])
			printf(" ");
		i++;
	}
	if (newline)
		printf("\n");
	return (0);
}

/* cd [dir]  - ohne Argument nach $HOME. Aktualisiert PWD und OLDPWD. */
static int	builtin_cd(char **argv, t_shell *sh)
{
	char	*dir;
	char	cwd[4096];

	if (argv[1] && argv[2])
		return (print_error("cd", "too many arguments"), 2);
	dir = argv[1] ? argv[1] : env_get(sh->env, "HOME");
	if (!dir)
		return (print_error("cd", "HOME not set"), 1);
	if (chdir(dir) != 0)
	{
		fprintf(stderr, "minishell: cd: %s: %s\n", dir, strerror(errno));
		return (1);
	}
	if (env_get(sh->env, "PWD"))
		env_set(&sh->env, "OLDPWD", env_get(sh->env, "PWD"));
	if (getcwd(cwd, sizeof(cwd)))
		env_set(&sh->env, "PWD", cwd);
	return (0);
}

static int	builtin_pwd(void)
{
	char	cwd[4096];

	if (!getcwd(cwd, sizeof(cwd)))
		return (perror("minishell: pwd"), 1);
	printf("%s\n", cwd);
	return (0);
}

int	run_builtin(char **argv, t_shell *sh)
{
	int	status;

	status = 0;
	if (strcmp(argv[0], "echo") == 0)
		status = builtin_echo(argv);
	else if (strcmp(argv[0], "cd") == 0)
		status = builtin_cd(argv, sh);
	else if (strcmp(argv[0], "pwd") == 0)
		status = builtin_pwd();
	else if (strcmp(argv[0], "export") == 0)
		status = builtin_export(argv, sh);
	else if (strcmp(argv[0], "unset") == 0)
		status = builtin_unset(argv, sh);
	else if (strcmp(argv[0], "env") == 0)
		status = builtin_env(sh);
	else if (strcmp(argv[0], "exit") == 0)
		status = builtin_exit(argv, sh);
	/* printf puffert - vor Umleitung zuruecksetzen/fork alles rausschreiben */
	fflush(stdout);
	return (status);
}
