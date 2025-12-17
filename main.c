#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
	TOKEN_NUMBER,
	TOKEN_OPERATOR,
	TOKEN_UNARY_OPERATOR,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_NULL
} token_type;

typedef struct
{
	token_type type;
	char* value;
	int priority;
} token;

token make_token(token_type type, const char* text)
{
	token t;
	t.type = type;
	t.priority = 0;
	if (text)
	{
		t.value = malloc(strlen(text) + 1);
		if (t.value)
		{
			strcpy(t.value, text);
		}
	}
	else
	{
		t.value = NULL;
	}

	if (type == TOKEN_UNARY_OPERATOR)
	{
		t.priority = 1;
	}
	else if (type == TOKEN_OPERATOR)
	{
		if (t.value)
		{
			if (strcmp(t.value, "**") == 0)
			{
				t.priority = 2;
			}
			else if (strcmp(t.value, "*") == 0 || strcmp(t.value, "/") == 0 || strcmp(t.value, "%") == 0)
			{
				t.priority = 3;
			}
			else if (strcmp(t.value, "+") == 0 || strcmp(t.value, "-") == 0)
			{
				t.priority = 4;
			}
			else if (strcmp(t.value, "<<") == 0 || strcmp(t.value, ">>") == 0)
			{
				t.priority = 5;
			}
			else if (strcmp(t.value, "&") == 0)
			{
				t.priority = 6;
			}
			else if (strcmp(t.value, "^") == 0)
			{
				t.priority = 7;
			}
			else if (strcmp(t.value, "|") == 0)
			{
				t.priority = 8;
			}
			else
			{
				t.priority = 100;
			}
		}
	}
	return t;
}

void free_token(token* t)
{
	if (!t)
	{
		return;
	}
	if (t->value)
	{
		free(t->value);
		t->value = NULL;
	}
	t->type = TOKEN_NULL;
	t->priority = 0;
}

token copy_token(const token* t)
{
	return make_token(t->type, t->value);
}

bool is_digit_char(char c)
{
	return (c >= '0' && c <= '9');
}

typedef struct
{
	token* data;
	size_t top;
	size_t capacity;
} stack;

bool initialize_stack(stack* s, size_t capacity)
{
	s->data = malloc(sizeof(token) * capacity);
	if (!s->data)
	{
		fprintf(stderr, "Error: Stack allocation failed\n");
		return false;
	}
	s->capacity = capacity;
	s->top = 0;
	return true;
}

void delete_stack(stack* s)
{
	if (!s || !s->data)
	{
		return;
	}
	for (size_t i = 0; i < s->top; i++)
	{
		free_token(&s->data[i]);
	}
	free(s->data);
	s->data = NULL;
	s->top = 0;
	s->capacity = 0;
}

bool push_stack(stack* s, token input)
{
	if (s->top >= s->capacity)
	{
		size_t newcap = s->capacity == 0 ? 8 : s->capacity * 2;
		token* tmp = realloc(s->data, sizeof(token) * newcap);
		if (!tmp)
		{
			fprintf(stderr, "Error: StackOverflow\n");
			return false;
		}
		s->data = tmp;
		s->capacity = newcap;
	}
	s->data[s->top++] = input;
	return true;
}

token pop_stack(stack* s)
{
	if (s->top == 0)
	{
		return make_token(TOKEN_NULL, NULL);
	}
	s->top--;
	return s->data[s->top];
}

token peek_stack(stack* s)
{
	if (s->top == 0)
	{
		return make_token(TOKEN_NULL, NULL);
	}
	return s->data[s->top - 1];
}

bool is_empty_stack(stack* s)
{
	return s->top == 0;
}

typedef struct
{
	token* data;
	size_t front;
	size_t rear;
	size_t capacity;
} queue;

bool initialize_queue(queue* q, size_t capacity)
{
	q->data = malloc(sizeof(token) * capacity);
	if (!q->data)
	{
		fprintf(stderr, "Error: Queue allocation failed\n");
		return false;
	}
	q->capacity = capacity;
	q->front = 0;
	q->rear = 0;
	return true;
}

void delete_queue(queue* q)
{
	if (!q || !q->data)
	{
		return;
	}
	for (size_t i = 0; i < q->rear; i++)
	{
		free_token(&q->data[i]);
	}
	free(q->data);
	q->data = NULL;
	q->front = q->rear = q->capacity = 0;
}

bool push_queue(queue* q, token input)
{
	if (q->rear >= q->capacity)
	{
		size_t newcap = q->capacity == 0 ? 8 : q->capacity * 2;
		token* tmp = realloc(q->data, sizeof(token) * newcap);
		if (!tmp)
		{
			fprintf(stderr, "Error: QueueOverflow\n");
			return false;
		}
		q->data = tmp;
		q->capacity = newcap;
	}
	q->data[q->rear++] = input;
	return true;
}

token pop_queue(queue* q)
{
	if (q->front == q->rear)
	{
		return make_token(TOKEN_NULL, NULL);
	}
	token res = q->data[q->front];
	q->data[q->front].value = NULL;
	q->front++;
	return res;
}

bool is_empty_queue(queue* q)
{
	return q->front == q->rear;
}

bool is_right_assoc(const token* t)
{
	if (!t || !t->value)
	{
		return false;
	}
	if (t->type == TOKEN_UNARY_OPERATOR)
	{
		return true;
	}
	return false;
}

bool is_operator_token(const token* t)
{
	return t && (t->type == TOKEN_OPERATOR || t->type == TOKEN_UNARY_OPERATOR);
}

bool tokenizator(char* math_expression, queue* res_queue, int* err_code)
{
	size_t length = strlen(math_expression);
	size_t index = 0;

	token prev = make_token(TOKEN_NULL, NULL);

	while (index < length)
	{
		if (math_expression[index] == ' ' || math_expression[index] == '\t' || math_expression[index] == '\n' ||
			math_expression[index] == '\r')
		{
			index++;
			continue;
		}

		if (is_digit_char(math_expression[index]))
		{
			size_t temp_index = index;
			while (temp_index < length && is_digit_char(math_expression[temp_index]))
			{
				temp_index++;
			}

			size_t buffer_size = temp_index - index + 1;
			char* buffer = malloc(buffer_size);
			if (!buffer)
			{
				if (err_code)
				{
					*err_code = 5;
					free_token(&prev);
					return false;
				}
			}
			memcpy(buffer, math_expression + index, buffer_size - 1);
			buffer[buffer_size - 1] = '\0';

			token temp_token = make_token(TOKEN_NUMBER, buffer);
			free(buffer);

			if (!push_queue(res_queue, temp_token))
			{
				free_token(&temp_token);
				free_token(&prev);
				if (err_code)
				{
					*err_code = 5;
				}
				return false;
			}

			index = temp_index;

			free_token(&prev);
			prev = copy_token(&temp_token);
			continue;
		}

		if (math_expression[index] == '(')
		{
			token tmp = make_token(TOKEN_LPAREN, "(");
			if (!push_queue(res_queue, tmp))
			{
				free_token(&tmp);
				free_token(&prev);
				if (err_code)
				{
					*err_code = 5;
					return false;
				}
			}
			free_token(&prev);
			prev = copy_token(&tmp);
			index++;
			continue;
		}

		if (math_expression[index] == ')')
		{
			token tmp = make_token(TOKEN_RPAREN, ")");
			if (!push_queue(res_queue, tmp))
			{
				free_token(&tmp);
				free_token(&prev);
				if (err_code)
				{
					*err_code = 5;
					return false;
				}
			}
			free_token(&prev);
			prev = copy_token(&tmp);
			index++;
			continue;
		}

		if (index + 1 < length)
		{
			if (math_expression[index] == '*' && math_expression[index + 1] == '*')
			{
				token tmp = make_token(TOKEN_OPERATOR, "**");
				if (!push_queue(res_queue, tmp))
				{
					free_token(&tmp);
					free_token(&prev);
					if (err_code)
					{
						*err_code = 5;
						return false;
					}
				}
				free_token(&prev);
				prev = copy_token(&tmp);
				index += 2;
				continue;
			}

			if (math_expression[index] == '>' && math_expression[index + 1] == '>')
			{
				token tmp = make_token(TOKEN_OPERATOR, ">>");
				if (!push_queue(res_queue, tmp))
				{
					free_token(&tmp);
					free_token(&prev);
					if (err_code)
					{
						*err_code = 5;
						return false;
					}
				}
				free_token(&prev);
				prev = copy_token(&tmp);
				index += 2;
				continue;
			}

			if (math_expression[index] == '<' && math_expression[index + 1] == '<')
			{
				token tmp = make_token(TOKEN_OPERATOR, "<<");
				if (!push_queue(res_queue, tmp))
				{
					free_token(&tmp);
					free_token(&prev);
					if (err_code)
					{
						*err_code = 5;
						return false;
					}
				}
				free_token(&prev);
				prev = copy_token(&tmp);
				index += 2;
				continue;
			}
		}

		char ch = math_expression[index];
		if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '&' || ch == '^' || ch == '|' || ch == '~')
		{
			bool unary = false;

			if ((ch == '+' || ch == '-' || ch == '~') &&
				(prev.type == TOKEN_NULL || is_operator_token(&prev) || prev.type == TOKEN_UNARY_OPERATOR || prev.type == TOKEN_LPAREN))
			{
				unary = true;
			}

			char tmp_s[2] = { ch, '\0' };
			token tmp = make_token(unary ? TOKEN_UNARY_OPERATOR : TOKEN_OPERATOR, tmp_s);

			if (!push_queue(res_queue, tmp))
			{
				free_token(&tmp);
				free_token(&prev);
				if (err_code)
				{
					*err_code = 5;
					return false;
				}
			}
			free_token(&prev);
			prev = copy_token(&tmp);

			index++;
			continue;
		}

		if (err_code)
		{
			*err_code = 1;
		}
		free_token(&prev);
		return false;
	}

	free_token(&prev);
	return true;
}

bool safe_pow(int a, int b, int* res)
{
	if (b < 0)
	{
		return false;
	}
	int64_t r = 1;
	for (int i = 0; i < b; i++)
	{
		r = (int64_t)((int32_t)r * (int32_t)a);
	}
	*res = (int32_t)r;
	return true;
}

bool binary_operators_operations(int32_t a, int32_t b, const char* op, int32_t* res, int* err_code)
{
	if (!op || !res)
	{
		if (err_code)
		{
			*err_code = 5;
		}
		return false;
	}

	if (strcmp(op, "+") == 0)
	{
		int64_t tmp = (int64_t)a + (int64_t)b;
		*res = (int32_t)tmp;
	}
	else if (strcmp(op, "-") == 0)
	{
		int64_t tmp = (int64_t)a - (int64_t)b;
		*res = (int32_t)tmp;
	}
	else if (strcmp(op, "*") == 0)
	{
		int64_t tmp = (int64_t)a * (int64_t)b;
		*res = (int32_t)tmp;
	}
	else if (strcmp(op, "**") == 0)
	{
		if (!safe_pow(a, b, res))
		{
			if (err_code)
			{
				*err_code = 3;
			}
			return false;
		}
	}
	else if (strcmp(op, "/") == 0)
	{
		if (b == 0 || (a == INT_MIN && b == -1))
		{
			if (err_code)
			{
				*err_code = 3;
			}
			return false;
		}
		*res = a / b;
	}
	else if (strcmp(op, "%") == 0)
	{
		if (b == 0)
		{
			if (err_code)
			{
				*err_code = 3;
			}
			return false;
		}
		*res = a % b;
	}
	else if (strcmp(op, "<<") == 0)
	{
		if (b < 0 || b >= 32)
		{
			if (err_code)
			{
				*err_code = 3;
			}
			return false;
		}
		*res = (int32_t)(a << b);
	}
	else if (strcmp(op, ">>") == 0)
	{
		if (b < 0 || b >= 32)
		{
			if (err_code)
			{
				*err_code = 3;
			}
			return false;
		}
		*res = (int32_t)(a >> b);
	}
	else if (strcmp(op, "&") == 0)
	{
		*res = a & b;
	}
	else if (strcmp(op, "^") == 0)
	{
		*res = a ^ b;
	}
	else if (strcmp(op, "|") == 0)
	{
		*res = a | b;
	}
	else
	{
		if (err_code)
		{
			*err_code = 1;
		}
		return false;
	}

	return true;
}

bool unary_operators_operations(int32_t a, const char* operation, int32_t* res, int* err_code)
{
	if (strcmp(operation, "~") == 0)
	{
		*res = ~a;
	}
	else if (strcmp(operation, "+") == 0)
	{
		*res = +a;
	}
	else if (strcmp(operation, "-") == 0)
	{
		*res = -a;
	}
	else
	{
		if (err_code)
		{
			*err_code = 1;
		}
		return false;
	}
	return true;
}

queue shunting_yard_algorithm(queue input, int* err_code)
{
	queue output;
	initialize_queue(&output, input.capacity ? input.capacity : 16);

	stack operator_stack;
	initialize_stack(&operator_stack, input.capacity ? input.capacity : 16);

	while (!is_empty_queue(&input))
	{
		token t = pop_queue(&input);
		if (t.type == TOKEN_NUMBER)
		{
			token copy = copy_token(&t);
			push_queue(&output, copy);
		}
		else if (t.type == TOKEN_OPERATOR)
		{
			while (!is_empty_stack(&operator_stack))
			{
				token top = peek_stack(&operator_stack);
				if (top.type == TOKEN_LPAREN)
				{
					break;
				}
				if (is_operator_token(&top))
				{
					if (top.type == TOKEN_UNARY_OPERATOR)
					{
						token popped = pop_stack(&operator_stack);
						token copy = copy_token(&popped);
						free_token(&popped);
						push_queue(&output, copy);
						continue;
					}
					bool should_pop = false;
					if (is_right_assoc(&t))
					{
						should_pop = (top.priority < t.priority);
					}
					else
					{
						if (top.priority < t.priority)
						{
							should_pop = true;
						}
						else if (top.priority == t.priority)
						{
							should_pop = !is_right_assoc(&top);
						}
						else
						{
							should_pop = false;
						}
					}
					if (should_pop)
					{
						token popped = pop_stack(&operator_stack);
						token copy = copy_token(&popped);
						free_token(&popped);
						push_queue(&output, copy);
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			token copy_op = copy_token(&t);
			push_stack(&operator_stack, copy_op);
		}
		else if (t.type == TOKEN_LPAREN)
		{
			token copy = copy_token(&t);
			push_stack(&operator_stack, copy);
		}
		else if (t.type == TOKEN_RPAREN)
		{
			bool found_lparen = false;
			while (!is_empty_stack(&operator_stack))
			{
				token top = peek_stack(&operator_stack);
				if (top.type == TOKEN_LPAREN)
				{
					found_lparen = true;
					token lparen = pop_stack(&operator_stack);
					free_token(&lparen);
					break;
				}
				else
				{
					token popped = pop_stack(&operator_stack);
					token copy = copy_token(&popped);
					free_token(&popped);
					push_queue(&output, copy);
				}
			}
			if (!found_lparen)
			{
				if (err_code)
				{
					*err_code = 2;
				}
				delete_queue(&output);
				delete_stack(&operator_stack);
				return output;
			}
		}
		else if (t.type == TOKEN_UNARY_OPERATOR)
		{
			token copy_unary = copy_token(&t);
			push_stack(&operator_stack, copy_unary);
		}
		else
		{
			if (err_code)
			{
				*err_code = 1;
			}
			delete_queue(&output);
			delete_stack(&operator_stack);
			delete_queue(&input);
			return output;
		}
	}

	while (!is_empty_stack(&operator_stack))
	{
		token top = pop_stack(&operator_stack);
		if (top.type == TOKEN_LPAREN || top.type == TOKEN_RPAREN)
		{
			free_token(&top);
			if (err_code)
			{
				*err_code = 2;
			}
			delete_queue(&output);
			delete_stack(&operator_stack);
			delete_queue(&input);
			return output;
		}
		token copy = copy_token(&top);
		free_token(&top);
		push_queue(&output, copy);
	}
	delete_stack(&operator_stack);
	return output;
}

bool calculate_expression(queue* q, int* out, int* err_code)
{
	stack st;
	if (!initialize_stack(&st, q->capacity))
	{
		if (err_code)
		{
			*err_code = 5;
		}
		return false;
	}

	while (!is_empty_queue(q))
	{
		token cur = pop_queue(q);

		if (cur.type == TOKEN_NUMBER)
		{
			token copy = copy_token(&cur);
			push_stack(&st, copy);
			continue;
		}

		if (cur.type == TOKEN_OPERATOR)
		{
			if (st.top < 2)
			{
				if (err_code)
				{
					*err_code = 2;
				}
				goto error;
			}

			token tb = pop_stack(&st);
			token ta = pop_stack(&st);

			char* endp = NULL;
			long lb = strtol(tb.value, &endp, 10);
			if (endp == tb.value || *endp != '\0' || lb < INT_MIN || lb > INT_MAX)
			{
				free_token(&tb);
				free_token(&ta);
				if (err_code)
				{
					*err_code = 1;
				}
				goto error;
			}
			endp = NULL;
			long la = strtol(ta.value, &endp, 10);
			if (endp == ta.value || *endp != '\0' || la < INT_MIN || la > INT_MAX)
			{
				free_token(&tb);
				free_token(&ta);
				if (err_code)
				{
					*err_code = 1;
				}
				goto error;
			}
			int32_t b = (int32_t)lb;
			int32_t a = (int32_t)la;

			free_token(&tb);
			free_token(&ta);

			int32_t r;
			if (!binary_operators_operations(a, b, cur.value, &r, err_code))
			{
				goto error;
			}

			char buf[32];
			snprintf(buf, sizeof(buf), "%d", r);
			push_stack(&st, make_token(TOKEN_NUMBER, buf));
		}
		else if (cur.type == TOKEN_UNARY_OPERATOR)
		{
			if (st.top < 1)
			{
				if (err_code)
				{
					*err_code = 2;
				}
				goto error;
			}

			token ta = pop_stack(&st);
			char* endp = NULL;
			long la = strtol(ta.value, &endp, 10);
			if (endp == ta.value || *endp != '\0' || la < INT_MIN || la > INT_MAX)
			{
				free_token(&ta);
				if (err_code)
				{
					*err_code = 1;
				}
				goto error;
			}
			int32_t a = (int32_t)la;
			free_token(&ta);

			int32_t r;
			if (!unary_operators_operations(a, cur.value, &r, err_code))
			{
				goto error;
			}

			char buf[32];
			snprintf(buf, sizeof(buf), "%d", r);
			push_stack(&st, make_token(TOKEN_NUMBER, buf));
		}
		else
		{
			if (err_code)
			{
				*err_code = 1;
			}
			goto error;
		}
	}

	if (st.top != 1)
	{
		if (err_code)
		{
			*err_code = 2;
		}
		goto error;
	}

	token res = pop_stack(&st);
	char* endp = NULL;
	long lres = strtol(res.value, &endp, 10);
	if (endp == res.value || *endp != '\0' || lres < INT_MIN || lres > INT_MAX)
	{
		free_token(&res);
		if (err_code)
		{
			*err_code = 1;
		}
		delete_stack(&st);
		return false;
	}
	*out = (int32_t)lres;
	free_token(&res);
	delete_stack(&st);
	return true;

error:
	delete_stack(&st);
	return false;
}

bool parse_console_data(int argc, char* argv[], char** input_file_path, char** output_file_path, bool* polish_notation)
{
	if (argc < 5 || argc > 6)
	{
		fprintf(stderr, "Error: incorrect amount of arguments. Usage: %s -i input_file -o output_file [-p]\n", argv[0]);
		return false;
	}

	*input_file_path = NULL;
	*output_file_path = NULL;
	*polish_notation = false;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-i") == 0)
		{
			if (i + 1 >= argc)
			{
				fprintf(stderr, "Error: missing input file name after -i\n");
				return false;
			}
			*input_file_path = argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-o") == 0)
		{
			if (i + 1 >= argc)
			{
				fprintf(stderr, "Error: missing output file name after -o\n");
				return false;
			}
			*output_file_path = argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-p") == 0)
		{
			*polish_notation = true;
		}
		else
		{
			fprintf(stderr, "Error: unknown argument %s\n", argv[i]);
			return false;
		}
	}

	if (*input_file_path == NULL)
	{
		fprintf(stderr, "Error: no input file provided\n");
		return false;
	}

	if (*output_file_path == NULL)
	{
		fprintf(stderr, "Error: no output file provided\n");
		return false;
	}

	return true;
}

bool parse_file_data(FILE* input_file, char** math_expression)
{
	if (!input_file)
	{
		return false;
	}
	if (fseek(input_file, 0, SEEK_END) != 0)
	{
		return false;
	}
	long size = ftell(input_file);
	if (size < 0)
	{
		return false;
	}
	rewind(input_file);

	*math_expression = malloc((size_t)size + 1);
	if (*math_expression == NULL)
	{
		return false;
	}

	size_t read = fread(*math_expression, 1, (size_t)size, input_file);
	(*math_expression)[read] = '\0';

	return true;
}

void print_queue_to_file(queue* q, FILE* out)
{
	for (size_t i = q->front; i < q->rear; i++)
	{
		fprintf(out, "%s\n", q->data[i].value);
	}
}

void print_answer_to_file(int answer, FILE* output_file)
{
	fprintf(output_file, "%d", answer);
}

int main(int argc, char* argv[])
{
	char* input_file_path = NULL;
	char* output_file_path = NULL;
	bool polish_notation = false;

	if (!parse_console_data(argc, argv, &input_file_path, &output_file_path, &polish_notation))
	{
		return 1;
	}

	FILE* input_file = fopen(input_file_path, "r");
	if (!input_file)
	{
		fprintf(stderr, "Error: Cannot open input file\n");
		return 5;
	}

	FILE* output_file = fopen(output_file_path, "w");
	if (!output_file)
	{
		fprintf(stderr, "Error: Cannot open output file\n");
		fclose(input_file);
		return 5;
	}

	char* expr = NULL;
	if (!parse_file_data(input_file, &expr))
	{
		fprintf(stderr, "Error: Cannot read input file\n");
		fclose(input_file);
		return 5;
	}

	int err_code = 0;

	queue tokenized_expression;
	if (!initialize_queue(&tokenized_expression, 100))
	{
		fprintf(stderr, "Error: Cannot initialize tokenized expression queue\n");
		fclose(input_file);
		free(expr);
		return 5;
	}
	if (!tokenizator(expr, &tokenized_expression, &err_code))
	{
		fprintf(stderr, "Error: Unsupported token\n");
		delete_queue(&tokenized_expression);
		fclose(input_file);
		free(expr);
		return err_code ? err_code : 1;
	}

	queue shunted_expression = shunting_yard_algorithm(tokenized_expression, &err_code);

	delete_queue(&tokenized_expression);
	if (err_code)
	{
		fprintf(stderr, "Error: Parse failed\n");
		delete_queue(&shunted_expression);
		fclose(input_file);
		fclose(output_file);
		free(expr);
		return err_code;
	}

	if (polish_notation)
	{
		print_queue_to_file(&shunted_expression, output_file);
		delete_queue(&shunted_expression);
	}
	else
	{
		int res;
		if (!calculate_expression(&shunted_expression, &res, &err_code))
		{
			fprintf(stderr, "Error: Evaluation failed\n");
			delete_queue(&shunted_expression);
			fclose(input_file);
			fclose(output_file);
			free(expr);
			return err_code ? err_code : 3;
		}
		print_answer_to_file(res, output_file);
		delete_queue(&shunted_expression);
	}
	free(expr);
	return 0;
}
