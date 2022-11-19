#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,TK_DIGIT,

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"\\(",'('},
  {"[0-9]",TK_DIGIT},     //digits
  {"-",'-'},
  {"\\*",'*'},
  {"/",'/'},
  {"\\)",')'},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};
/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

enum error_state{illegal = -2,legal};//two different error states
enum bracket_state{normal = -4,unnormal};//Ok,I try my best ,it's my only method to resolve expression like "((?+?)*(?+?))"
int bracket_state_flag = 0;//this flag is just to detect "(())"
int error_state_flag = 0;
bool check_balanced_brackets(int p,int q)//check the brackets is balanced
{
	int lo = 0;	
	//printf("in check_balanced_brackets before run p is %d,q is %d\n",p,q);
	for(int i = p;i <= q;i++)
	{
		if(tokens[i].type == ')'&&lo == 0)
		{
			continue;
		}
		if(tokens[i].type == '(')
		{
			lo++;
		}
		if(tokens[i].type == ')'&&lo != 0)
		{
			lo--;
		}	
	}	
		if(lo !=  0)
		{
//			printf("in check_balanced_brackets after run before return false p is %d,q is %d\n",p,q);
				return false;
		}
	//	printf("in check_balanced_brackets after run before return true p is %d,q is %d\n",p,q);

	return true;
}
bool check_parentheses(int p,int q)
{
	//	printf("in check_parentheses before run,p is %d,q is %d\n",p,q);
	if(check_balanced_brackets(p,q) == true)//brackets are balanced but not sure the whole is "()" 
	{
		if(p == 0)
		{
			if(check_balanced_brackets(p+1,q-1) == true)//to check expressions like "()*()"
			{
				if(tokens[p+1].type == '('&&tokens[q-1].type == ')')
					bracket_state_flag = normal;
				error_state_flag = legal;//sub-expression can be calculated
	//			printf("in check_parentheses() if1_1 p is %d,q is %d\n",p,q);
				return true;
			}
			else{
				error_state_flag = legal;
	//			printf("in check_parentheses() if1_2 p is %d,q is %d\n",p,q);
				return false;
			}
		}	
		else if(tokens[p].type == '('&&tokens[q].type == ')')
		{
			if(bracket_state_flag == normal)
			{
				bracket_state_flag = unnormal;
				return false;
			}		
			error_state_flag = legal;
	//		printf("in check_parentheses() if2 p is %d,q is %d\n",p,q);
			return true;
		}
		else{
			error_state_flag = legal;

	//		printf("in check_parentheses() if3 p is %d,q is %d\n",p,q);
			return false;
		}
	}
	else{
	//	printf("brackets not balanced when p is %d,q is %d\n",p,q);
		error_state_flag = illegal;//bad expression
		return false;
	}	
	return false;
}
word_t eval(int p,int q)
{
	if(p > q)return 0;
	else if(p == q)
	{
		return (word_t)(tokens[p].str[0]-'0');
	}
	else if(check_parentheses(p,q) == true)
	{
		return eval(p+1,q-1);
	}
	else{
		if(error_state_flag == illegal)
		{
			printf("p = %d,q = %d\n",p,q);
			return -1;
		}
		int op = 0;
		int priority = -1;
		for(int i = p;i <= q;i++)//find op place
		{

			if(i <=q&&tokens[i].type == '(')
			{
				while(i <= q&&tokens[i].type != ')')i++;	
			}
			else if(tokens[i].type == '+'||tokens[i].type == '-')
			{
				op = i;
				priority = 1;
			}
			else if((tokens[i].type == '*'||tokens[i].type == '/')&&priority <= 0)
			{
				op = i;
				priority = 0;
			}
			else if(error_state_flag == legal&&tokens[i].type == ')')
			{
				printf("in eval() i == %d \n",i);
				op = -1;
				priority = -1;
			}
		}
		int val1 = eval(p,op-1);
		
		int val2 = eval(op+1,q);
	//	printf("in eval():p = %d,q = %d,op = %d,val1 = %d,val2 = %d\n",p,q,op,val1,val2);
		switch(tokens[op].type)
		{
			case '+':return val1+val2;
			case '-':return val1-val2;

			case '*':return val1*val2;
			case '/':return val1/val2;
			default:assert(0);	 
		}
	}
}
static bool make_token(char *e) {
  int position = 0;
  int pre_position = 0;
  int i;
  regmatch_t pmatch;


  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
		pre_position = position;
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
		int len = sizeof(e[pre_position]);
		//printf("len:%d,e[pre_position] = %c\n",len,e[pre_position]);
        switch (rules[i].token_type) {
		  case '+':tokens[pre_position].type = rules[i].token_type;nr_token++;break; 
		  case TK_DIGIT: tokens[pre_position].type = rules[i].token_type;
						nr_token++; 
						memcpy(tokens[pre_position].str,&e[pre_position],len);

//		printf("tokens[%d].str:%s\n",pre_position,myToken[pre_position].str);
		break; 

		  case '(': tokens[pre_position].type = rules[i].token_type;nr_token++;break; 

		  case '-': tokens[pre_position].type = rules[i].token_type;nr_token++;break; 
		  
		  case '*':tokens[pre_position].type = rules[i].token_type;nr_token++;break; 

		  case ')':tokens[pre_position].type = rules[i].token_type;nr_token++;break; 

		  case '/':tokens[pre_position].type = rules[i].token_type;nr_token++;break; 

      //   default: TODO();
	  	
        }
		/*for(int i = 0;i < 5;i++)
		{
			printf("tokens[%d] is %d\n",i,tokens[i].type);
		}*/
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
//  TODO();
//	printf("in expr(),e is %s\n",e);
	int p = 0,q = strlen(e)-1;
//	printf("in expr(),q equals %d\n",q);
	return	eval(p,q);
  return 0;
}
