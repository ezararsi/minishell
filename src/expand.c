/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 13:43:02 by ezararsi          #+#    #+#             */
/*   Updated: 2026/11/05 19:14:31 by ezararsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/* s[*i] zeigt auf das '$'. Gibt den Wert der Variable zurueck (malloc). */
static char	*get_variable(char *s, int *i, t_shell *sh)
{
	int		start;
	char	*name;
	char	*value;

	(*i)++;
	if (s[*i] == '?')
	{
		(*i)++;
		value = safe_malloc(12);
		snprintf(value, 12, "%d", sh->exit_status);
		return (value);
	}
	/* $1, $2 ... gibt es in unserer Shell nicht -> leer (wie bash ohne
	** Argumente). Es wird nur EINE Ziffer verbraucht: $12 -> "" + "2" */
	if (s[*i] >= '0' && s[*i] <= '9')
	{
		(*i)++;
		return (safe_strdup(""));
	}
	start = *i;
	while (s[*i] == '_' || (s[*i] >= 'a' && s[*i] <= 'z')
		|| (s[*i] >= 'A' && s[*i] <= 'Z') || (s[*i] >= '0' && s[*i] <= '9'))
		(*i)++;
	name = safe_strndup(s + start, *i - start);
	value = env_get(sh->env, name);
	free(name);
	return (safe_strdup(value ? value : ""));
}

static bool	is_var_start(char c)
{
	return (c == '?' || c == '_' || (c >= 'a' && c <= 'z')
		|| (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'));
}

/*
** Laenge des naechsten Stuecks "normaler" Text ab s[i], also bis zum
** naechsten Zeichen, das eine Sonderbehandlung braucht ($ oder Quote).
** Dieses Stueck wird dann auf einmal angehaengt statt Zeichen fuer
** Zeichen - das spart viele malloc/free-Aufrufe.
*/
static int	plain_len(char *s, int i, char quote, bool quotes)
{
	int	start;

	start = i;
	while (s[i] && s[i] != '$')
	{
		if (quotes && !quote && (s[i] == '\'' || s[i] == '"'))
			break ;
		if (quotes && quote && s[i] == quote)
			break ;
		i++;
	}
	return (i - start);
}

/*
** vars:   $VAR und $? ersetzen
** quotes: ' und " auswerten und entfernen
**
** In '...' wird nichts ersetzt, in "..." schon:
**   echo '$HOME'  ->  $HOME
**   echo "$HOME"  ->  /home/enes
*/
char	*expand(char *s, t_shell *sh, bool vars, bool quotes)
{
	char	*res;
	char	*value;
	char	quote;
	int		i;
	int		len;

	res = safe_strdup("");
	quote = 0;
	i = 0;
	while (s[i])
	{
		if (quotes && !quote && (s[i] == '\'' || s[i] == '"'))
			quote = s[i++];
		else if (quotes && quote && s[i] == quote)
		{
			quote = 0;
			i++;
		}
		else if (vars && s[i] == '$' && quote != '\'' && is_var_start(s[i + 1]))
		{
			value = get_variable(s, &i, sh);
			res = join_free(res, value);
			free(value);
		}
		else
		{
			len = plain_len(s, i, quote, quotes);
			if (len == 0)
				len = 1;
			res = join_n(res, s + i, len);
			i += len;
		}
	}
	return (res);
}
