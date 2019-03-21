/**
 *  \file
 *
 *  Definitions for command entries and arrays of them.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */
#ifndef _COMMAND_H__
#define _COMMAND_H__

#include "ld_comp_array.h"
#include "types.h"

/** \brief Execute command.
 *
 *  \param[in]  pObj      Pointer to command private data.
 *  \param[in]  argstr    Argument(s).
 *  \param[in]  pOut      Implementation specific object used
 *                        to print command output.
 *
 *  \return     RESULT_OK if operation performed successfully. Error message
 * otherwise.
 */
typedef result_t (*Command_func)(void *pObj, const char *argstr, void *);

typedef struct CommandDelegate_struct {
  /** Function implementing the command or NULL for node having child nodes. */
  Command_func func;

  /** Pointer passed to func. */
  void *pObj;
} CommandDelegate;

typedef struct Command_struct Command;

typedef struct CommandArray_struct {
  /** Pointer to first entry in array of commands. */
  Command *commands;

  /** Pointer to optional (default) entry in array of commands.
   *  May be NULL when there's no such entry.
   */
  Command *defaultCommand;
} CommandArray;

struct Command_struct {
  /** Command name or NULL for a terminating entry. */
  immutable_str name;

  union {
    /** Defines command if isLeaf == TRUE. */
    CommandDelegate command;

    /** Defines array of commands under this node if isLeaf == FALSE. */
    CommandArray subcommands;
  } body;

  /** FALSE if the command has subnodes. */
  bool isLeaf;
};

#define COMMAND_NAME(_pEntry)                                                  \
  ((immutable_str)READ_IMMUTABLE_PTR(&((_pEntry)->name)))
#define COMMAND_ISLEAF(_pEntry)                                                \
  ((bool)READ_IMMUTABLE_BYTE(&((_pEntry)->isLeaf)))
#define COMMAND_FUNC(_pEntry)                                                  \
  ((Command_func)READ_IMMUTABLE_PTR(&((_pEntry)->body.command.func)))
#define COMMAND_POBJ(_pEntry)                                                  \
  ((void *)READ_IMMUTABLE_PTR(&((_pEntry)->body.command.pObj)))
#define COMMAND_SUBCMDS(_pEntry)                                               \
  ((Command *)READ_IMMUTABLE_PTR(&((_pEntry)->body.subcommands.commands)))
#define COMMAND_DEFAULT(_pEntry)                                               \
  ((Command *)READ_IMMUTABLE_PTR(&((_pEntry)->body.subcommands.defaultCommand)))

#define COMMAND_ARRAY_DECLARE(_name) LD_COMP_ARRAY_DECLARE(_name, Command, {0})
#define COMMAND_ARRAY_EXTERN(_name) LD_COMP_ARRAY_EXTERN(_name, Command)

#define COMMAND_IN_ARRAY(_arrayName) LD_COMP_ARRAY(_arrayName)

#define DEFINE_COMMAND(_path, _name, _pObj, _pObjName, _argsName, _pOutName)   \
  COMMAND_ARRAY_EXTERN(commandArray_##_path);                                  \
  static IMMUTABLE_STR(commandName_##_path##_##_name) = #_name;                \
  static result_t command_##_path##_##_name##_execute(                         \
      void *_pObjName, const char *_argsName, void *_pOutName);                \
  Command command_##_path##_##_name COMMAND_IN_ARRAY(commandArray_##_path) = { \
      .name = commandName_##_path##_##_name,                                   \
      .isLeaf = true,                                                          \
      .body.command.func = &command_##_path##_##_name##_execute,               \
      .body.command.pObj = _pObj};                                             \
  static result_t command_##_path##_##_name##_execute(                         \
      void *_pObjName, const char *_argsName, void *_pOutName)

#define DEFINE_COMMAND_NAME_FNC_PTR(_path, _name, _pObj, _name_str, _fnc_ptr)  \
  COMMAND_ARRAY_EXTERN(commandArray_##_path);                                  \
  Command command_##_path##_##_name COMMAND_IN_ARRAY(commandArray_##_path) = { \
      .name = _name_str,                                                       \
      .isLeaf = true,                                                          \
      .body.command.func = _fnc_ptr,                                           \
      .body.command.pObj = _pObj}

#define DEFINE_COMMAND_FNC_PTR(_path, _name, _pObj, _fnc_ptr)                  \
  COMMAND_ARRAY_EXTERN(commandArray_##_path);                                  \
  static IMMUTABLE_STR(commandName_##_path##_##_name) = #_name;                \
  Command command_##_path##_##_name COMMAND_IN_ARRAY(commandArray_##_path) = { \
      .name = commandName_##_path##_##_name,                                   \
      .isLeaf = true,                                                          \
      .body.command.func = _fnc_ptr,                                           \
      .body.command.pObj = _pObj}

#define DEFINE_COMMAND_ARRAY(_path, _name, _default)                           \
  COMMAND_ARRAY_DECLARE(commandArray_##_path##_##_name);                       \
  static IMMUTABLE_STR(commandName_##_name) = #_name;                          \
  Command command_##_path##_##_name COMMAND_IN_ARRAY(commandArray_##_path) = { \
      .name = commandName_##_name,                                             \
      .isLeaf = false,                                                         \
      .body.subcommands.commands = commandArray_##_path##_##_name,             \
      .body.subcommands.defaultCommand =                                       \
          &command_##_path##_##_name##_##_default}

#define DEFINE_COMMAND_ARRAY_NAMESTR(_path, _name, _default, _namestr)         \
  COMMAND_ARRAY_DECLARE(commandArray_##_path##_##_name);                       \
  Command command_##_path##_##_name COMMAND_IN_ARRAY(commandArray_##_path) = { \
      .name = _namestr,                                                        \
      .isLeaf = false,                                                         \
      .body.subcommands.commands = commandArray_##_path##_##_name,             \
      .body.subcommands.defaultCommand =                                       \
          &command_##_path##_##_name##_##_default}

#endif // !_COMMAND_H__
