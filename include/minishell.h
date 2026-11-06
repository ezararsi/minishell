/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 18:49:12 by ezararsi          #+#    #+#             */
/*   Updated: 2026/11/05 19:49:36 by ezararsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <string.h>
# include <unistd.h>
# include <fcntl.h>
# include <errno.h>
# include <signal.h>
# include <limits.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <readline/readline.h>
# include <readline/history.h>

/* einzige globale Variable: das zuletzt empfangene Signal */
extern volatile sig_atomic_t	g_signal;

typedef enum e_type
{
	WORD,
	PIPE,
	REDIR_IN,
	REDIR_OUT,
	APPEND,
	HEREDOC
}	t_type;

/* Ergebnis des Lexers: "ls -l | wc" -> [ls] [-l] [|] [wc] */
typedef struct s_token
{
	t_type			type;
	char			*value;
	struct s_token	*next;
}	t_token;

typedef struct s_redir
{
	t_type			type;
	char			*file;		/* Dateiname bzw. Heredoc-Delimiter */
	bool			expand;		/* Heredoc: $VAR ersetzen? */
	int				fd;			/* Heredoc: fd der Temp-Datei */
	struct s_redir	*next;
}	t_redir;

/* Ein Befehl einer Pipeline. "ls | wc" -> zwei t_cmd */
typedef struct s_cmd
{
	char			**argv;
	t_redir			*redirs;
	struct s_cmd	*next;
}	t_cmd;

typedef struct s_env
{
	char			*key;
	char			*value;
	struct s_env	*next;
}	t_env;

typedef struct s_shell
{
	t_env	*env;
	int		exit_status;
	bool	should_exit;
	bool	is_child;
}	t_shell;

/* lexer.c */
t_token	*lexer(char *line, bool *error);
void	free_tokens(t_token *tok);

/* parser.c */
t_cmd	*parser(t_token *tok, t_shell *sh);
void	free_cmds(t_cmd *cmd);

/* expand.c */
char	*expand(char *s, t_shell *sh, bool vars, bool quotes);

/* heredoc.c */
int		handle_heredocs(t_cmd *cmds, t_shell *sh);

/* redirect.c */
int		apply_redirs(t_redir *r);

/* exec.c / path.c */
void	execute(t_cmd *cmds, t_shell *sh);
char	*find_path(char *cmd, t_shell *sh);

/* builtins.c / builtins2.c */
bool	is_builtin(char *name);
int		run_builtin(char **argv, t_shell *sh);
int		builtin_export(char **argv, t_shell *sh);
int		builtin_unset(char **argv, t_shell *sh);
int		builtin_env(t_shell *sh);
int		builtin_exit(char **argv, t_shell *sh);

/* env.c */
t_env	*env_init(char **envp);
char	*env_get(t_env *env, char *key);
void	env_set(t_env **env, char *key, char *value);
void	env_unset(t_env **env, char *key);
char	**env_to_array(t_env *env);
void	env_free(t_env *env);

/* input.c */
bool	is_interactive(void);
char	*read_input(const char *prompt);

/* signals.c */
void	signals_prompt(void);
void	signals_ignore(void);
void	signals_default(void);

/* utils.c */
void	*safe_malloc(size_t size);
char	*safe_strdup(const char *s);
char	*safe_strndup(const char *s, size_t n);
char	*join_n(char *s1, const char *s2, size_t n);
char	*join_free(char *s1, char *s2);
void	free_array(char **arr);
void	print_error(char *name, char *msg);

#endif
