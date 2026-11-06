/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins2.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 15:24:27 by ezararsi          #+#    #+#             */
/*   Updated: 2026/11/05 16:18:10 by ezararsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/* Gueltiger Variablenname: Buchstabe oder _, danach auch Ziffern */
static bool	is_valid_name(char *s, int len)
{
	int	i;

	if (len == 0 || !(s[0] == '_' || (s[0] >= 'a' && s[0] <= 'z')
			|| (s[0] >= 'A' && s[0] <= 'Z')))
		return (false);
	i = 1;
	while (i < len)
	{
		if (!(s[i] == '_' || (s[i] >= 'a' && s[i] <= 'z')
				|| (s[i] >= 'A' && s[i] <= 'Z')
				|| (s[i] >= '0' && s[i] <= '9')))
			return (false);
		i++;
	}
	return (true);
}

/* export            -> alle Variablen anzeigen
** export KEY=VALUE  -> setzen
** export KEY        -> ohne Wert anlegen */
int	builtin_export(char **argv, t_shell *sh)
{
	t_env	*e;
	char	*eq;
	char	*key;
	int		status;
	int		i;

	if (!argv[1])
	{
		for (e = sh->env; e; e = e->next)
		{
			if (e->value)
				printf("declare -x %s=\"%s\"\n", e->key, e->value);
			else
				printf("declare -x %s\n", e->key);
		}
		return (0);
	}
	status = 0;
	i = 0;
	while (argv[++i])
	{
		eq = strchr(argv[i], '=');
		key = eq ? safe_strndup(argv[i], eq - argv[i]) : safe_strdup(argv[i]);
		if (!is_valid_name(key, strlen(key)))
		{
			fprintf(stderr, "minishell: export: `%s': not a valid "
				"identifier\n", argv[i]);
			status = 1;
		}
		else
			env_set(&sh->env, key, eq ? eq + 1 : NULL);
		free(key);
	}
	return (status);
}

int	builtin_unset(char **argv, t_shell *sh)
{
	int	i;

	i = 0;
	while (argv[++i])
		env_unset(&sh->env, argv[i]);
	return (0);
}

/* env zeigt nur Variablen mit Wert (anders als export) */
int	builtin_env(t_shell *sh)
{
	t_env	*e;

	for (e = sh->env; e; e = e->next)
		if (e->value)
			printf("%s=%s\n", e->key, e->value);
	return (0);
}

/*
** Wandelt s in einen Exit-Code (0-255) um. Gibt false zurueck, wenn s
** keine gueltige Zahl ist oder nicht in einen long long passt
** (z.B. "exit 99999999999999999999") - dann meldet exit einen Fehler.
**
** Ueberlaufpruefung VOR dem Multiplizieren: n * 10 + digit darf nicht
** groesser als limit werden, also muss n <= (limit - digit) / 10 sein.
*/
static bool	parse_exit_code(char *s, int *code)
{
	unsigned long long	n;
	unsigned long long	limit;
	int					digit;
	int					i;
	bool				negative;

	i = 0;
	negative = (s[0] == '-');
	if (s[0] == '-' || s[0] == '+')
		i++;
	if (!s[i])
		return (false);
	limit = negative ? (unsigned long long)LLONG_MAX + 1 : LLONG_MAX;
	n = 0;
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (false);
		digit = s[i] - '0';
		if (n > (limit - digit) / 10)
			return (false);
		n = n * 10 + digit;
		i++;
	}
	/* Exit-Codes sind nur 8 Bit: -1 -> 255, 256 -> 0 */
	*code = (unsigned char)(negative ? 0 - n : n);
	return (true);
}

int	builtin_exit(char **argv, t_shell *sh)
{
	int	code;

	code = 0;
	if (!sh->is_child)
		fprintf(stderr, "exit\n");
	if (argv[1] && !parse_exit_code(argv[1], &code))
	{
		fprintf(stderr, "minishell: exit: %s: numeric argument required\n",
			argv[1]);
		sh->should_exit = true;
		return (2);
	}
	if (argv[1] && argv[2])
		return (print_error("exit", "too many arguments"), 2);
	sh->should_exit = true;
	if (argv[1])
		return (code);
	return (sh->exit_status);
}
