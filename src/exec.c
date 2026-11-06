/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 14:08:10 by ezararsi          #+#    #+#             */
/*   Updated: 2026/11/05 18:04:53 by ezararsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/* Laeuft im Kindprozess: Umleitungen setzen, dann Programm starten */
static void	exec_child(t_cmd *cmd, t_shell *sh)
{
	char		*path;
	char		**envp;
	struct stat	st;
	int			err;

	sh->is_child = true;
	signals_default();
	if (apply_redirs(cmd->redirs) < 0)
		exit(1);
	if (!cmd->argv)
		exit(0);
	if (is_builtin(cmd->argv[0]))
		exit(run_builtin(cmd->argv, sh));
	path = find_path(cmd->argv[0], sh);
	if (!path)
	{
		print_error(cmd->argv[0], "command not found");
		exit(127);
	}
	if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
	{
		print_error(cmd->argv[0], "Is a directory");
		exit(126);
	}
	envp = env_to_array(sh->env);
	execve(path, cmd->argv, envp);
	/* hier kommt man nur hin, wenn execve fehlschlaegt.
	** errno sichern - fprintf in print_error darf es veraendern. */
	err = errno;
	print_error(cmd->argv[0], strerror(err));
	exit(err == ENOENT ? 127 : 126);
}

/*
** Builtins wie cd, export, exit muessen im Shell-Prozess selbst laufen -
** in einem Kindprozess wuerde z.B. cd nur das Kind betreffen.
** stdin/stdout werden gesichert und nach dem Builtin wiederhergestellt.
*/
static void	run_builtin_parent(t_cmd *cmd, t_shell *sh)
{
	int	saved_in;
	int	saved_out;

	saved_in = dup(STDIN_FILENO);
	saved_out = dup(STDOUT_FILENO);
	if (saved_in < 0 || saved_out < 0)
	{
		perror("minishell: dup");
		if (saved_in >= 0)
			close(saved_in);
		sh->exit_status = 1;
		return ;
	}
	if (apply_redirs(cmd->redirs) == 0)
		sh->exit_status = run_builtin(cmd->argv, sh);
	else
		sh->exit_status = 1;
	dup2(saved_in, STDIN_FILENO);
	dup2(saved_out, STDOUT_FILENO);
	close(saved_in);
	close(saved_out);
}

/* Wartet auf alle Kinder. Exit-Status ist der des letzten Befehls. */
static int	wait_children(pid_t last_pid, int count)
{
	int		status;
	int		result;
	pid_t	pid;

	result = 0;
	while (count-- > 0)
	{
		pid = wait(&status);
		if (pid != last_pid)
			continue ;
		if (WIFEXITED(status))
			result = WEXITSTATUS(status);
		else if (WIFSIGNALED(status))
		{
			result = 128 + WTERMSIG(status);
			if (WTERMSIG(status) == SIGINT)
				write(STDOUT_FILENO, "\n", 1);
			else if (WTERMSIG(status) == SIGQUIT)
				write(STDERR_FILENO, "Quit (core dumped)\n", 19);
		}
	}
	return (result);
}

/* Kindprozess: stdin/stdout mit den Pipes verbinden, dann ausfuehren */
static void	child_process(t_cmd *cmd, t_shell *sh, int prev_fd, int fd[2])
{
	if (prev_fd != -1)
	{
		dup2(prev_fd, STDIN_FILENO);
		close(prev_fd);
	}
	if (cmd->next)
	{
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
	}
	exec_child(cmd, sh);
}

/*
** Fuehrt eine Pipeline aus:  cmd1 | cmd2 | cmd3
**
**   cmd1 --pipe1--> cmd2 --pipe2--> cmd3
**
** Fuer jeden Befehl wird ein Kindprozess erzeugt. stdout eines Befehls
** wird per dup2 auf das Schreib-Ende der Pipe gelegt, stdin des naechsten
** auf das Lese-Ende. Nicht benutzte Enden muessen geschlossen werden,
** sonst bekommt z.B. wc nie ein EOF und wartet ewig.
**
** Schlaegt pipe() oder fork() fehl (z.B. zu viele Prozesse), wird
** abgebrochen - aber auf die schon gestarteten Kinder trotzdem gewartet,
** sonst bleiben sie als "Zombie-Prozesse" zurueck.
*/
void	execute(t_cmd *cmds, t_shell *sh)
{
	t_cmd	*cmd;
	int		fd[2];
	int		prev_fd;
	int		count;
	pid_t	pid;

	if (!cmds->next && cmds->argv && is_builtin(cmds->argv[0]))
	{
		run_builtin_parent(cmds, sh);
		return ;
	}
	signals_ignore();
	prev_fd = -1;
	count = 0;
	pid = -1;
	cmd = cmds;
	while (cmd)
	{
		if (cmd->next && pipe(fd) < 0)
		{
			perror("minishell: pipe");
			break ;
		}
		pid = fork();
		if (pid < 0)
		{
			perror("minishell: fork");
			if (cmd->next)
			{
				close(fd[0]);
				close(fd[1]);
			}
			break ;
		}
		if (pid == 0)
			child_process(cmd, sh, prev_fd, fd);
		if (prev_fd != -1)
			close(prev_fd);
		prev_fd = -1;
		if (cmd->next)
		{
			close(fd[1]);
			prev_fd = fd[0];
		}
		count++;
		cmd = cmd->next;
	}
	if (prev_fd != -1)
		close(prev_fd);
	sh->exit_status = wait_children(pid, count);
	if (cmd)
		sh->exit_status = 1;
}
