/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 18:37:47 by ezararsi          #+#    #+#             */
/*   Updated: 2026/11/05 20:46:32 by ezararsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	add_token(t_token **list, t_type type, char *value)
{
	t_token	*new;
	t_token	*cur;

	new = safe_malloc(sizeof(t_token));
	new->type = type;
	new->value = value;
	new->next = NULL;
	if (!*list)
	{
		*list = new;
		return ;
	}
	cur = *list;
	while (cur->next)
		cur = cur->next;
	cur->next = new;
}

/* Erkennt | < > << >> und gibt die Laenge des Operators zurueck */
static int	read_operator(char *s, t_token **list)
{
	if (s[0] == '<' && s[1] == '<')
		return (add_token(list, HEREDOC, NULL), 2);
	if (s[0] == '>' && s[1] == '>')
		return (add_token(list, APPEND, NULL), 2);
	if (s[0] == '<')
		return (add_token(list, REDIR_IN, NULL), 1);
	if (s[0] == '>')
		return (add_token(list, REDIR_OUT, NULL), 1);
	return (add_token(list, PIPE, NULL), 1);
}

/*
** Liest ein Wort bis zum naechsten Leerzeichen oder Operator.
** Innerhalb von Quotes zaehlen Leerzeichen und | < > zum Wort.
** Die Quotes bleiben stehen - sie werden erst in expand.c entfernt.
*/
static int	read_word(char *s, t_token **list)
{
	int		i;
	char	quote;

	i = 0;
	while (s[i] && s[i] != ' ' && s[i] != '\t' && !strchr("|<>", s[i]))
	{
		if (s[i] == '\'' || s[i] == '"')
		{
			quote = s[i++];
			while (s[i] && s[i] != quote)
				i++;
			if (!s[i])
			{
				print_error("syntax error", "unclosed quote");
				return (-1);
			}
		}
		i++;
	}
	add_token(list, WORD, safe_strndup(s, i));
	return (i);
}

t_token	*lexer(char *line, bool *error)
{
	t_token	*list;
	int		i;
	int		len;

	list = NULL;
	*error = false;
	i = 0;
	while (line[i])
	{
		if (line[i] == ' ' || line[i] == '\t')
		{
			i++;
			continue ;
		}
		if (strchr("|<>", line[i]))
			len = read_operator(line + i, &list);
		else
			len = read_word(line + i, &list);
		if (len < 0)
		{
			*error = true;
			free_tokens(list);
			return (NULL);
		}
		i += len;
	}
	return (list);
}

void	free_tokens(t_token *tok)
{
	t_token	*next;

	while (tok)
	{
		next = tok->next;
		free(tok->value);
		free(tok);
		tok = next;
	}
}
