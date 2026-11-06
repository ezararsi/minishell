/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 13:37:59 by ezararsi          #+#    #+#             */
/*   Updated: 2026/11/05 14:33:00 by ezararsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static bool	is_redir(t_type type)
{
	return (type == REDIR_IN || type == REDIR_OUT
		|| type == APPEND || type == HEREDOC);
}

/* Prueft z.B. "| ls", "ls |", "ls | | wc", "cat <" */
static bool	check_syntax(t_token *tok)
{
	if (tok && tok->type == PIPE)
		return (print_error("syntax error", "unexpected token `|'"), false);
	while (tok)
	{
		if (tok->type == PIPE && (!tok->next || tok->next->type == PIPE))
			return (print_error("syntax error", "unexpected token `|'"),
				false);
		if (is_redir(tok->type) && (!tok->next || tok->next->type != WORD))
			return (print_error("syntax error",
					"unexpected token after redirection"), false);
		tok = tok->next;
	}
	return (true);
}

static void	add_arg(t_cmd *cmd, char *arg)
{
	int	count;

	count = 0;
	while (cmd->argv && cmd->argv[count])
		count++;
	cmd->argv = realloc(cmd->argv, sizeof(char *) * (count + 2));
	if (!cmd->argv)
		exit(1);
	cmd->argv[count] = arg;
	cmd->argv[count + 1] = NULL;
}

static void	add_redir(t_cmd *cmd, t_type type, char *word, t_shell *sh)
{
	t_redir	*new;
	t_redir	*cur;

	new = safe_malloc(sizeof(t_redir));
	new->type = type;
	new->fd = -1;
	new->next = NULL;
	/* Heredoc-Delimiter: nur Quotes entfernen. <<'EOF' -> keine Expansion */
	if (type == HEREDOC)
	{
		new->file = expand(word, sh, false, true);
		new->expand = !strchr(word, '\'') && !strchr(word, '"');
	}
	else
		new->file = expand(word, sh, true, true);
	if (!cmd->redirs)
	{
		cmd->redirs = new;
		return ;
	}
	cur = cmd->redirs;
	while (cur->next)
		cur = cur->next;
	cur->next = new;
}

static t_cmd	*new_cmd(void)
{
	t_cmd	*cmd;

	cmd = safe_malloc(sizeof(t_cmd));
	cmd->argv = NULL;
	cmd->redirs = NULL;
	cmd->next = NULL;
	return (cmd);
}

/*
** Tokens -> Liste von Befehlen. Jede Pipe startet einen neuen Befehl.
** "cat < in | grep x > out"  ->  [cat, <in] -> [grep x, >out]
*/
t_cmd	*parser(t_token *tok, t_shell *sh)
{
	t_cmd	*head;
	t_cmd	*cur;
	char	*arg;

	if (!check_syntax(tok))
		return (NULL);
	head = new_cmd();
	cur = head;
	while (tok)
	{
		if (tok->type == PIPE)
		{
			cur->next = new_cmd();
			cur = cur->next;
		}
		else if (is_redir(tok->type))
		{
			add_redir(cur, tok->type, tok->next->value, sh);
			tok = tok->next;
		}
		else
		{
			arg = expand(tok->value, sh, true, true);
			/* $LEER ohne Quotes ergibt kein Argument (wie in bash) */
			if (arg[0] == '\0' && !strchr(tok->value, '\'')
				&& !strchr(tok->value, '"'))
				free(arg);
			else
				add_arg(cur, arg);
		}
		tok = tok->next;
	}
	return (head);
}

void	free_cmds(t_cmd *cmd)
{
	t_cmd	*next;
	t_redir	*r;
	t_redir	*rnext;

	while (cmd)
	{
		next = cmd->next;
		free_array(cmd->argv);
		r = cmd->redirs;
		while (r)
		{
			rnext = r->next;
			if (r->fd >= 0)
				close(r->fd);
			free(r->file);
			free(r);
			r = rnext;
		}
		free(cmd);
		cmd = next;
	}
}
