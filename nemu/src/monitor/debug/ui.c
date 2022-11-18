#include <isa.h>
#include "expr.h"
#include "watchpoint.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);
int is_batch_mode();
void isa_reg_display();
paddr_t host_to_guest(void *addr);
word_t expr(char* e,bool * success);
/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args)
{
	char* cmd_char_si = strtok(args," ");
	uint64_t n = (uint64_t)cmd_char_si;
	cpu_exec(n);	
	return 0;
}
static int cmd_info(char *args)
{
	isa_reg_display();
	return 0;	
}
static int cmd_x(char * args)
{
	char * cmd_char_x = strtok(args," ");
	int n = strtol(cmd_char_x,NULL,10);
	cmd_char_x = strtok(NULL," ");

	paddr_t addr = host_to_guest(cmd_char_x); 
	for(int i = 0;i < n;i++)
	{
		printf("memory: %#010x\n",addr);
		addr += 1;
	}
	return 0;
}
static int cmd_p(char * args)
{
	bool suc = true;
	bool *success = &suc;
	
	char * cmd_char_p = strtok(args," ");

	word_t flag = expr(cmd_char_p,success);
	return flag; 
}

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  {"si","Single step implentment",cmd_si},
  /* TODO: Add more commands */
  {"info","Display the value of register",cmd_info},
  {"x","prinf the memory",cmd_x},
  {"p","get the value of expression",cmd_p},
};


#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop() {
  if (is_batch_mode()) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
