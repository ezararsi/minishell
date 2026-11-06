/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 17:10:54 by ezararsi          #+#    #+#             */
/*   Updated: 2026/11/05 22:33:45 by ezararsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*env_get(t_env *env, char *key)
{
	while (env)
	{
		if (strcmp(env->key, key) == 0)
			return (env->value);
		env = env->next;
	}
	return (NULL);
}

/* value == NULL: Variable ohne Wert (z.B. "export X") */
void	env_set(t_env **env, char *key, char *value)
{
	t_env	*cur;
	t_env	*new;

	cur = *env;
	while (cur)
	{
		if (strcmp(cur->key, key) == 0)
		{
			if (value)
			{
				free(cur->value);
				cur->value = safe_strdup(value);
			}
			return ;
		}
		if (!cur->next)
			break ;
		cur = cur->next;
	}
	new = safe_malloc(sizeof(t_env));
	new->key = safe_strdup(key);
	new->value = value ? safe_strdup(value) : NULL;
	new->next = NULL;
	if (cur)
		cur->next = new;
	else
		*env = new;
}

void	env_unset(t_env **env, char *key)
{
	t_env	*cur;
	t_env	*prev;

	cur = *env;
	prev = NULL;
	while (cur)
	{
		if (strcmp(cur->key, key) == 0)
		{
			if (prev)
				prev->next = cur->next;
			else
				*env = cur->next;
			free(cur->key);
			free(cur->value);
			free(cur);
			return ;
		}
		prev = cur;
		cur = cur->next;
	}
}

/* envp aus main in eine eigene Liste kopieren, SHLVL erhoehen */
t_env	*env_init(char **envp)
{
	t_env	*env;
	char	*eq;
	char	*key;
	char	buf[16];
	int		i;

	env = NULL;
	i = 0;
	while (envp[i])
	{
		eq = strchr(envp[i], '=');
		if (eq)
		{
			key = safe_strndup(envp[i], eq - envp[i]);
			env_set(&env, key, eq + 1);
			free(key);
		}
		i++;
	}
	i = env_get(env, "SHLVL") ? atoi(env_get(env, "SHLVL")) : 0;
	snprintf(buf, sizeof(buf), "%d", i + 1);
	env_set(&env, "SHLVL", buf);
	return (env);
}

/* fuer execve: Liste -> char** im Format "KEY=VALUE" */
char	**env_to_array(t_env *env)
{
	char	**arr;
	t_env	*cur;
	int		count;
	size_t	len;

	count = 0;
	for (cur = env; cur; cur = cur->next)
		if (cur->value)
			count++;
	arr = safe_malloc(sizeof(char *) * (count + 1));
	count = 0;
	for (cur = env; cur; cur = cur->next)
	{
		if (!cur->value)
			continue ;
		len = strlen(cur->key) + strlen(cur->value) + 2;
		arr[count] = safe_malloc(len);
		snprintf(arr[count], len, "%s=%s", cur->key, cur->value);
		count++;
	}
	arr[count] = NULL;
	return (arr);
}

void	env_free(t_env *env)
{
	t_env	*next;

	while (env)
	{
		next = env->next;
		free(env->key);
		free(env->value);
		free(env);
		env = next;
	}
}
