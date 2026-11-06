/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ezararsi <ezararsi@student.42vienna.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 16:27:35 by ezararsi          #+#    #+#             */
/*   Updated: 2026/11/05 19:29:23 by ezararsi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	*safe_malloc(size_t size)
{
	void	*ptr;

	ptr = malloc(size);
	if (!ptr)
	{
		perror("minishell: malloc");
		exit(1);
	}
	return (ptr);
}

/*
** strdup/strndup koennen wie malloc NULL zurueckgeben, wenn kein
** Speicher mehr frei ist. Statt das an jeder Stelle zu pruefen,
** kapseln wir die Pruefung hier - genau wie bei safe_malloc.
*/
char	*safe_strdup(const char *s)
{
	return (safe_strndup(s, strlen(s)));
}

char	*safe_strndup(const char *s, size_t n)
{
	char	*copy;

	copy = safe_malloc(n + 1);
	memcpy(copy, s, n);
	copy[n] = '\0';
	return (copy);
}

/* haengt die ersten n Zeichen von s2 an s1 an und gibt s1 frei */
char	*join_n(char *s1, const char *s2, size_t n)
{
	char	*res;
	size_t	len1;

	len1 = strlen(s1);
	res = safe_malloc(len1 + n + 1);
	memcpy(res, s1, len1);
	memcpy(res + len1, s2, n);
	res[len1 + n] = '\0';
	free(s1);
	return (res);
}

/* haengt s2 komplett an s1 an und gibt s1 frei */
char	*join_free(char *s1, char *s2)
{
	return (join_n(s1, s2, strlen(s2)));
}

void	free_array(char **arr)
{
	int	i;

	if (!arr)
		return ;
	i = 0;
	while (arr[i])
		free(arr[i++]);
	free(arr);
}

/* Fehlermeldung im Format "minishell: name: msg" */
void	print_error(char *name, char *msg)
{
	fprintf(stderr, "minishell: %s: %s\n", name, msg);
}
