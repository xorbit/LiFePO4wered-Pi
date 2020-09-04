/* 
 * LiFePO4wered/Pi variable read/write command line tool
 * Copyright (C) 2015-2020 Patrick Van Oosterwijck
 * Released under the GPL v2
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "lifepo4wered-data.h"


/* Read or write operation */

enum eOperation {
  OP_INVALID,
  OP_READ,
  OP_WRITE
};

/* Decimal or hexadecimal data */

enum eDataFormat {
  DF_INVALID,
  DF_DEC,
  DF_HEX,
  DF_DATA
};

/* Variable not specified */

#define LFP_VAR_UNSPECIFIED     (LFP_VAR_INVALID + 1)

/* Print help */

void print_help(char *name, char *error, uint8_t access_mask) {
  if (error) {
    fprintf(stderr, "ERROR: %s\n\n", error);
  }
  if (access_mask & ACCESS_READ && access_mask & ACCESS_WRITE) {
    printf("Usage: %s <operation> <variable> [value]\n\n", name);
    printf("Available operations:\n");
    printf("READ or GET: get variable and print it in decimal\n");
    printf("READHEX, GETHEX or HEX: get variable and print it in hexadecimal\n");
    printf("WRITE, SET or PUT: set the variable to the provided value\n\n");
    printf("Available variables:\n");
  } else if (access_mask & ACCESS_READ) {
    printf("Available variables for READ:\n");
  } else if (access_mask & ACCESS_WRITE) {
    printf("Available variables for WRITE:\n");
  }
  for (int i=0; i<LFP_VAR_COUNT; i++) {
    if (access_lifepo4wered(i, access_mask)) {
      printf("\t%s\n", lifepo4wered_var_name[i]);
    }
  }
}

/* Capitalize a string */

void capitalize(char *s) {
  for (; *s; s++) {
    *s = toupper(*s);
  }
}

/* Get the operation (and optionally data format) from the command line
 * operation argument */

enum eOperation get_operation(char *op, enum eDataFormat *fmt) {
  struct sOpRef {
    const char *s;
    enum eOperation op;
    enum eDataFormat fmt;
  };
  struct sOpRef op_table[] = {
    { "READ",     OP_READ,  DF_DEC  },
    { "GET",      OP_READ,  DF_DEC  },
    { "READHEX",  OP_READ,  DF_HEX  },
    { "GETHEX",   OP_READ,  DF_HEX  },
    { "HEX",      OP_READ,  DF_HEX  },
    { "WRITE",    OP_WRITE, DF_DATA },
    { "SET",      OP_WRITE, DF_DATA },
    { "PUT",      OP_WRITE, DF_DATA },
  };
  capitalize(op);
  for (int i=0; i<sizeof(op_table)/sizeof(struct sOpRef); i++) {
    if (strcmp(op, op_table[i].s) == 0) {
      if (fmt) *fmt = op_table[i].fmt;
      return op_table[i].op;
    }
  }
  if (fmt) *fmt = DF_INVALID;
  return OP_INVALID;
}

/* Get the variable from the command line variable argument */

enum eLiFePO4weredVar get_variable(char *var) {
  capitalize(var);
  for (int i=0; i<LFP_VAR_COUNT; i++) {
    if (strcmp(var, lifepo4wered_var_name[i]) == 0) {
      return (enum eLiFePO4weredVar)i;
    }
  }
  return LFP_VAR_INVALID;
}

/* Program entry point */

int main(int argc, char *argv[]) {
  int32_t value = 0;

  if (argc < 2) {
    print_help(argv[0], "No operation specified", ACCESS_READ|ACCESS_WRITE);
    return 1;
  }

  enum eOperation op;
  enum eDataFormat fmt;
  enum eLiFePO4weredVar var;
  op = get_operation(argv[1], &fmt);

  if (op == OP_INVALID) {
    print_help(argv[0], "Invalid operation", ACCESS_READ|ACCESS_WRITE);
    return 2;
  }

  uint8_t access_mask = (op == OP_WRITE ? ACCESS_WRITE : 0) |
                        (op == OP_READ ? ACCESS_READ : 0);

  if (argc < 3) {
    var = LFP_VAR_UNSPECIFIED;
    if (op == OP_WRITE) {
      print_help(argv[0], "No variable specified", access_mask);
      return 3;
    }
  } else {
    var = get_variable(argv[2]);
  }

  if (var == LFP_VAR_INVALID) {
    print_help(argv[0], "Invalid variable name", access_mask);
    return 4;
  }

  if (op == OP_WRITE && argc < 4) {
    print_help(argv[0], "No write value specified", 0);
    return 5;
  }

  if (op == OP_READ) {
    if (var != LFP_VAR_UNSPECIFIED) {
      value = read_lifepo4wered(var);
      if (fmt == DF_DEC) {
        printf("%d\n", value);
      } else {
        printf("0x%04X\n", value);
      }
    } else {
      for (int i=0; i<LFP_VAR_COUNT; i++) {
        if (access_lifepo4wered(i, access_mask)) {
          value = read_lifepo4wered(i);
          if (fmt == DF_DEC) {
            printf("%s = %d\n", lifepo4wered_var_name[i], value);
          } else {
            printf("%s = 0x%04X\n", lifepo4wered_var_name[i], value);
          }
        }
      }
    }
  }

  if (op == OP_WRITE) {
    value = strtol(argv[3], NULL, 0);
    value = write_lifepo4wered(var, value);
    printf("%d\n", value);
  }

  return value == -1 || value == -2 ? 6 : 0;
}
