/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"


/* Set this flag to 1 to enable debug messages */
//#define ENABLE_DEBUG_MESSAGES 1

int ENABLE_DEBUG_MESSAGES = 1;
/*int showDisplayMessages(char * type)
{
  if (strcmp(type, "simulate") == 0)
  {
    ENABLE_DEBUG_MESSAGES = 0;
  }
}*/

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 *        implementation
 */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 16);
  memset(cpu->regs_valid, 0, sizeof(int) * 16);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

APEX_Instruction * create_noop(){
  APEX_Instruction * no_op = malloc(sizeof(APEX_Instruction));
  strcpy(no_op->opcode, "\0");
  no_op->rd = 0;
  no_op->rs1 = 0;
  no_op->rs2 = 0;
  return no_op;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 *        implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "STORE") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
    printf("[Contents=%d   RS2=%d   IMM=%d]", stage->rs1_value, stage->rs2_value, stage->imm);
  }

  if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
    printf("[R%d <- %d]", stage->rd, stage->buffer);
  }

  if (strcmp(stage->opcode, "ADD") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    printf("[Res=%d   RS1=%d   RS2=%d]", stage->buffer, stage->rs1_value, stage->rs2_value);
  }

  if (strcmp(stage->opcode, "ADDI") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
    printf("[Res=%d   RS1=%d   IMM=%d]", stage->buffer, stage->rs1_value, stage->imm);
  }

  if (strcmp(stage->opcode, "SUB") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    printf("[Res=%d   RS1=%d   RS2=%d]", stage->buffer, stage->rs1_value, stage->rs2_value);
  }

  if (strcmp(stage->opcode, "MUL") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    printf("[Res=%d   RS1=%d   RS2=%d]", stage->buffer, stage->rs1_value, stage->rs2_value);
  }

  if (strcmp(stage->opcode, "AND") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    printf("[Res=%d   RS1=%d   RS2=%d]", stage->buffer, stage->rs1_value, stage->rs2_value);
  }

  if (strcmp(stage->opcode, "OR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    printf("[Res=%d   RS1=%d   RS2=%d]", stage->buffer, stage->rs1_value, stage->rs2_value);
  }

  if (strcmp(stage->opcode, "EX-OR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    printf("[Res=%d   RS1=%d   RS2=%d]", stage->buffer, stage->rs1_value, stage->rs2_value);
  }

  if (strcmp(stage->opcode, "LOAD") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
    printf("[Res=%d   RS1=%d   IMM=%d]", stage->buffer, stage->rs1_value, stage->imm);
  }

  if (strcmp(stage->opcode, "LDR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    printf("[Res=%d   RS1=%d   RS2=%d]", stage->buffer, stage->rs1_value, stage->rs2_value);
  }

  if (strcmp(stage->opcode, "HALT") == 0) {
    printf("%s", stage->opcode);
    //printf("[Res=%d   RS1=%d   RS2=%d]", stage->buffer, stage->rs1_value, stage->rs2_value);
  }

  if (strcmp(stage->opcode, "JUMP") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rs1, stage->imm);
    printf("[PC Value=%d   RS1=%d   IMM=%d]", stage->buffer, stage->rs1_value, stage->imm);
  }

  if (strcmp(stage->opcode, "BZ") == 0) {
    printf("%s,#%d ", stage->opcode, stage->imm);
    printf("[PC Value=%d   IMM=%d]", stage->buffer, stage->imm);
  }

  if (strcmp(stage->opcode, "BNZ") == 0) {
    printf("%s,#%d ", stage->opcode, stage->imm);
    printf("[PC Value=%d   IMM=%d]", stage->buffer, stage->imm);
  }

}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

static void
print_stalled_content(char* name)
{
  printf("%-15s: Empty", name);
  printf(" ");
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *         implementation
 */
int isHalted = 0;
int
fetch(APEX_CPU* cpu)   
{

  CPU_Stage* stage = &cpu->stage[F];
  if (!stage->busy && !stage->stalled) {  
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;

    if(cpu->stage[DRF].stalled==1 ){

      if (ENABLE_DEBUG_MESSAGES) {
        print_stage_content("Fetch", stage);
      }

      return 0;
    }

    /* Update PC for next instruction */
    cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/
    cpu->stage[DRF] = cpu->stage[F];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Fetch", stage);
    }
  }
  else
  {
    cpu->stage[DRF] = cpu->stage[F];
    if (ENABLE_DEBUG_MESSAGES) {
      print_stalled_content("Fetch");
    }
  }
  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *         implementation
 */
int isBranched = 0;
int branchPrevVal = 1;
int
decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];

  if(cpu->stage[EX].busy==1 && strcmp(cpu->stage[EX].opcode, "MUL") == 0){
    stage->stalled=1;
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Decode/RF", stage);
    }
    return 0;
  }

  int isRS1Forwarded = 0;
  int isRS2Forwarded = 0;
  if (cpu->regs_valid[stage->rs1] == 1)
  {
    isRS1Forwarded = 1;
  }
  
  if(cpu->regs_valid[stage->rs2] == 1)
  {
    isRS2Forwarded = 1;
  }
  
  if (!stage->busy && !stage->stalled) {

    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    }

    if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 || strcmp(stage->opcode, "MUL") == 0 || strcmp(stage->opcode, "AND") == 0 || strcmp(stage->opcode, "OR") == 0 || strcmp(stage->opcode, "EX-OR") == 0 || strcmp(stage->opcode, "LDR") == 0 || strcmp(stage->opcode, "STORE") == 0) {
      if (isRS1Forwarded == 1)
      {
        if(strcmp(cpu->stage[EX].opcode, "LOAD") == 0){
          if(stage->rs2 == stage->rs1)
            stage->stalled = 1;
        }
        else{
          stage->rs1_value = cpu->forwarded_data[stage->rs1];
        }
      }
      else
      {
        stage->rs1_value = cpu->regs[stage->rs1];
      }

      if (isRS2Forwarded == 1)
      {
        stage->rs2_value = cpu->forwarded_data[stage->rs2];
      }
      else
      {
        stage->rs2_value = cpu->regs[stage->rs2];
      }
      /*stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];*/
    }

    if (strcmp(stage->opcode, "ADDI") == 0 || strcmp(stage->opcode, "LOAD") == 0 || strcmp(stage->opcode, "JUMP") == 0) {
      if (isRS1Forwarded == 1)
      {
        stage->rs1_value = cpu->forwarded_data[stage->rs1];
      }
      else
      {
        stage->rs1_value = cpu->regs[stage->rs1];
      }
    }

    if (strcmp(stage->opcode, "HALT") == 0) {
      int index = get_code_index(cpu->stage->pc);

      for (int i = index+1; i < cpu->code_memory_size; i++)
      {
        cpu->code_memory[i] = *create_noop();
        //strcpy(cpu->code_memory[i].opcode, "\0");
      }
    }

    if (strcmp(stage->opcode, "BZ") == 0 || strcmp(stage->opcode, "BNZ") == 0) {
      if(isBranched != 2){
        isBranched = 1;
        //cpu->stage[EX].isBranched = 1;
        //stage->stalled = 1;
      }
      else if(isBranched == 2)
      {
        isBranched = 0;
      } 
    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[EX] = cpu->stage[DRF];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Decode/RF", stage);
    }
  }
  else{
    cpu->stage[EX] = cpu->stage[DRF];
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Decode/RF", stage);
    }
  }
  return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *         implementation
 */

int
execute(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX];

  int isMul = 0;
  if(stage->busy && strcmp(stage->opcode, "MUL") == 0){
    cpu->stage[EX].busy--;
    isMul = 1;
  }

  if (!stage->busy && !stage->stalled && isMul == 0) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      stage->rs2_value = cpu->forwarded_data[stage->rs2];
      stage->mem_address = stage->rs2_value + stage->imm;
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer = stage->imm;
      //Copy data into intermediate array for forwarding data
      cpu->forwarded_data[stage->rd] = stage->buffer;

    }

    if (strcmp(stage->opcode, "ADD") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer = stage->rs1_value + stage->rs2_value;
      cpu->forwarded_data[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "ADDI") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer = stage->rs1_value + stage->imm;
      cpu->forwarded_data[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "SUB") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer = stage->rs1_value - stage->rs2_value;
      cpu->forwarded_data[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "MUL") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer = stage->rs1_value * stage->rs2_value;
      //cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
      stage->busy=1;
      cpu->forwarded_data[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "AND") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer = stage->rs1_value & stage->rs2_value;
      cpu->forwarded_data[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "OR") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer = stage->rs1_value | stage->rs2_value;
      cpu->forwarded_data[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "EX-OR") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer = stage->rs1_value ^ stage->rs2_value;
      cpu->forwarded_data[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer = stage->rs1_value + stage->imm;
      
    }

    if (strcmp(stage->opcode, "LDR") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer = stage->rs1_value + stage->rs2_value;
      cpu->forwarded_data[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "HALT") == 0) {
    }

    if (strcmp(stage->opcode, "JUMP") == 0) {
      strcpy(cpu->stage[DRF].opcode,"\0");
      cpu->stage[DRF].stalled = 1;
      strcpy(cpu->stage[F].opcode,"\0");
      cpu->stage[F].stalled = 1;
      
      cpu->pc = stage->buffer = stage->rs1_value + stage->imm;
      if(stage->imm > 0)
      {
        cpu->ins_completed += (((stage->buffer - (stage->pc+4))/4));
      }
      else
      {
        cpu->ins_completed -= ((((stage->pc+4) - stage->buffer)/4));
      }
    }

    if (strcmp(stage->opcode, "BZ") == 0) {
      if(branchPrevVal == 0){
        cpu->pc = stage->buffer = stage->pc + stage->imm;
        if(stage->imm > 0)
        {
          cpu->ins_completed += (((stage->buffer - stage->pc)/4)+2);
        }
        else
        {
          cpu->ins_completed -= (((stage->pc - stage->buffer)/4)+2);
        }
         
        strcpy(cpu->stage[DRF].opcode, "\0");
        strcpy(cpu->stage[DRF].opcode, "\0");
      }
    }

    if (strcmp(stage->opcode, "BNZ") == 0) {
      if(branchPrevVal != 0){
        cpu->pc = stage->buffer = stage->pc + stage->imm;
        if(stage->imm > 0)
        {
          cpu->ins_completed += (((stage->pc - stage->buffer)/4)+2);
        }
        else
        {
          cpu->ins_completed -= (((stage->pc - stage->buffer)/4)+2);
        }
         
        strcpy(cpu->stage[DRF].opcode, "\0");
        strcpy(cpu->stage[DRF].opcode, "\0");
      }
    }

  if(strcmp(cpu->stage[DRF].opcode, "BZ") == 0)
  {
    branchPrevVal = stage->buffer;
  }

    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[MEM] = cpu->stage[EX];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute", stage);
    }
  }
  else
  {
      cpu->stage[MEM] = cpu->stage[EX];
      if (isMul)
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        cpu->regs_valid[stage->rd] = 1;
        cpu->forwarded_data[stage->rd] = stage->buffer;
        if (ENABLE_DEBUG_MESSAGES) {
          print_stage_content("Execute", stage);
        }
      }
      else
      {
        if (ENABLE_DEBUG_MESSAGES) {
          print_stalled_content("Execute");
        }
      }
  }

  return 0;
}

/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *         implementation
 */
int
memory(APEX_CPU* cpu)
{


  CPU_Stage* stage = &cpu->stage[MEM];
  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      stage->rs1_value = cpu->forwarded_data[stage->rs1];
      cpu->data_memory[stage->mem_address] = stage->rs1_value;
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {
      stage->buffer = cpu->data_memory[stage->buffer];
      cpu->forwarded_data[stage->rd] = stage->buffer;
      cpu->stage[DRF].stalled = 0;
    }

    if (strcmp(stage->opcode, "LDR") == 0) {
      stage->buffer = cpu->data_memory[stage->buffer];
    }

    if (strcmp(stage->opcode, "JUMP") == 0) {
      cpu->stage[DRF].stalled = 0;
      cpu->stage[F].stalled = 0;
    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[WB] = cpu->stage[MEM];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory", stage);
    }

  }
  else{
    cpu->stage[WB] = cpu->stage[MEM];
    if (ENABLE_DEBUG_MESSAGES) {
      print_stalled_content("Memory");
    }
    
  }
  
  return 0;
}

/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *         implementation
 */
int
writeback(APEX_CPU* cpu)
{

  CPU_Stage* stage = &cpu->stage[WB];

  if (!stage->busy && !stage->stalled) {
    
    /* Update register file */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    /* Update register file */
    if (strcmp(stage->opcode, "ADD") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "ADDI") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "SUB") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "MUL") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "AND") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "OR") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "EX-OR") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "LDR") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "HALT") == 0) {
      isHalted = 1;
    }
    
    if (strcmp(stage->opcode, "JUMP") == 0) {
      
    }

    cpu->ins_completed++;
    cpu->regs_valid[stage->rd] = 0;

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Writeback", stage);
    }

  }
  
  else{
    if(isBranched == 1)
    {
      isBranched = 2;
      cpu->stage[DRF].stalled = 0;
    }
    if (ENABLE_DEBUG_MESSAGES) {
      print_stalled_content("Writeback");
    }
  }
  
   return 0;
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 *         implementation
 */
void display(APEX_CPU* cpu)
{
  printf("\n\n\n\tSTATE OF ARCHITECTURAL REGISTER FILE\n");
  for (int i = 0; i < 16; i++)
  {
    printf("| REG[%02d] | Value = %-9d | Status = %-9s |\n", i, cpu->regs[i], cpu->regs_valid[i] == 0 ? "VALID" : "INVALID");
  }
  printf("\n");

  printf("\n\n\n\tSTATE OF DATA MEMORY\n");
  for (int i = 0; i < 100; i++)
  {
    printf("| MEM[%02d] | Data Value = %02d |\n", i, cpu->data_memory[i]);
  }
  printf("\n");
}

int
APEX_cpu_run(APEX_CPU* cpu, const char * type, int cycles)
{
  if (strcmp(type, "simulate") == 0)
  {
    ENABLE_DEBUG_MESSAGES = 0;
  }

  int cycleCount = 0;
  while (cycleCount < cycles) {

    /* All the instructions committed, so exit */
    if (cpu->ins_completed == cpu->code_memory_size || isHalted) {
      printf("\n\n\n(apex) >> Simulation Complete\n");

      
      break;
    }

    if (ENABLE_DEBUG_MESSAGES) {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", (cpu->clock+1));
      printf("--------------------------------\n");

    }

    writeback(cpu);
    memory(cpu);
    execute(cpu);
    decode(cpu);
    fetch(cpu);
    cpu->clock++;

    cycleCount++;
  }
  display(cpu);
  return 0;
}
