#include "php_snuffleupagus.h"

ZEND_DECLARE_MODULE_GLOBALS(snuffleupagus)

static int parse_enable(char *line, bool * restrict retval, bool * restrict simulation) {
  bool enable = false, disable = false;
  sp_config_functions sp_config_funcs[] = {
    {parse_empty, SP_TOKEN_ENABLE, &(enable)},
    {parse_empty, SP_TOKEN_DISABLE, &(disable)},
    {parse_empty, SP_TOKEN_SIMULATION, simulation},
    {0}};

  int ret = parse_keywords(sp_config_funcs, line);

  if (0 != ret) {
    return ret;
  }

  if (!(enable ^ disable)) {
    sp_log_err("config", "A rule can't be enabled and disabled on line %zu.", sp_line_no);
    return -1;
  } 

  *retval = enable;
  
  return ret;
}

int parse_random(char *line) {
  return parse_enable(line, &(SNUFFLEUPAGUS_G(config).config_random->enable), NULL);
}

int parse_disable_xxe(char *line) {
  return parse_enable(line, &(SNUFFLEUPAGUS_G(config).config_disable_xxe->enable), NULL);
}

int parse_auto_cookie_secure(char *line) {
  return parse_enable(line, &(SNUFFLEUPAGUS_G(config).config_auto_cookie_secure->enable), NULL);
}

int parse_global_strict(char *line) {
  return parse_enable(line, &(SNUFFLEUPAGUS_G(config).config_global_strict->enable), NULL);
}

int parse_unserialize(char *line) {
  return parse_enable(line, &(SNUFFLEUPAGUS_G(config).config_unserialize->enable), &(SNUFFLEUPAGUS_G(config).config_unserialize->simulation));
}

int parse_readonly_exec(char *line) {
  return parse_enable(line, &(SNUFFLEUPAGUS_G(config).config_readonly_exec->enable), &(SNUFFLEUPAGUS_G(config).config_readonly_exec->simulation));
}

int parse_global(char *line) {
  sp_config_functions sp_config_funcs_encryption_key[] = {
      {parse_str, SP_TOKEN_ENCRYPTION_KEY,
       &(SNUFFLEUPAGUS_G(config).config_snuffleupagus->encryption_key)},
      {0}};
  return parse_keywords(sp_config_funcs_encryption_key, line);
}

int parse_cookie_encryption(char *line) {
  int ret = 0;
  char *name = NULL;

  sp_config_functions sp_config_funcs_cookie_encryption[] = {
      {parse_str, SP_TOKEN_NAME, &name},
      {parse_int, SP_TOKEN_MASK_IPV4,
       &(SNUFFLEUPAGUS_G(config).config_cookie_encryption->mask_ipv4)},
      {parse_int, SP_TOKEN_MASK_IPV6,
       &(SNUFFLEUPAGUS_G(config).config_cookie_encryption->mask_ipv6)},
      {0}};

  ret = parse_keywords(sp_config_funcs_cookie_encryption, line);
  if (0 != ret) {
    return ret;
  }

  if (32 < SNUFFLEUPAGUS_G(config).config_cookie_encryption->mask_ipv4) {
    SNUFFLEUPAGUS_G(config).config_cookie_encryption->mask_ipv4 = 32;
  }
  if (128 < SNUFFLEUPAGUS_G(config).config_cookie_encryption->mask_ipv6) {
    SNUFFLEUPAGUS_G(config).config_cookie_encryption->mask_ipv6 = 128;
  }

  if (name) {
    zend_hash_str_add_empty_element(
        SNUFFLEUPAGUS_G(config).config_cookie_encryption->names, name,
        strlen(name));
  }
  return SUCCESS;
}

int parse_disabled_functions(char *line) {
  int ret = 0;
  bool enable = true, disable = false;
  sp_disabled_function *df = pecalloc(sizeof(*df), 1, 1);

  sp_config_functions sp_config_funcs_disabled_functions[] = {
      {parse_empty, SP_TOKEN_ENABLE, &(enable)},
      {parse_empty, SP_TOKEN_DISABLE, &(disable)},
      {parse_str, SP_TOKEN_ALIAS, &(df->alias)},
      {parse_empty, SP_TOKEN_SIMULATION, &(df->simulation)},
      {parse_str, SP_TOKEN_FILENAME, &(df->filename)},
      {parse_regexp, SP_TOKEN_FILENAME_REGEXP, &(df->r_filename)},
      {parse_str, SP_TOKEN_FUNCTION, &(df->function)},
      {parse_regexp, SP_TOKEN_FUNCTION_REGEXP, &(df->r_function)},
      {parse_str, SP_TOKEN_DUMP, &(df->dump)},
      {parse_empty, SP_TOKEN_ALLOW, &(df->allow)},
      {parse_empty, SP_TOKEN_DROP, &(df->drop)},
      {parse_str, SP_TOKEN_HASH, &(df->hash)},
      {parse_str, SP_TOKEN_PARAM, &(df->param)},
      {parse_regexp, SP_TOKEN_VALUE_REGEXP, &(df->regexp)},
      {parse_str, SP_TOKEN_VALUE, &(df->value)},
      {parse_regexp, SP_TOKEN_PARAM_REGEXP, &(df->r_param)},
      {parse_php_type, SP_TOKEN_PARAM_TYPE, &(df->param_type)},
      {parse_str, SP_TOKEN_RET, &(df->ret)},
      {parse_cidr, SP_TOKEN_CIDR, &(df->cidr)},
      {parse_regexp, SP_TOKEN_RET_REGEXP, &(df->r_ret)},
      {parse_php_type, SP_TOKEN_RET_TYPE, &(df->ret_type)},
      {parse_str, SP_TOKEN_LOCAL_VAR, &(df->var)},
      {0}};

  ret = parse_keywords(sp_config_funcs_disabled_functions, line);

  if (0 != ret) {
    return ret;
  }

  if (true == disable){
    df->enable = false;
  } else {
    df->enable = true;
  }

  if (df->value && df->regexp) {
    sp_log_err("config",
               "Invalid configuration line: 'sp.disabled_functions%s':"
               "'.value' and '.regexp' are mutually exclusives on line %zu.",
               line, sp_line_no);
    return -1;
  } else if (df->r_function && df->function) {
    sp_log_err("config",
               "Invalid configuration line: 'sp.disabled_functions%s': "
               "'.r_function' and '.function' are mutually exclusive on line %zu.",
               line, sp_line_no);
    return -1;
  } else if (df->r_filename && df->filename) {
    sp_log_err("config",
               "Invalid configuration line: 'sp.disabled_functions%s':"
               "'.r_filename' and '.filename' are mutually exclusive on line %zu.",
               line, sp_line_no);
    return -1;
  } else if (df->r_param && df->param) {
    sp_log_err("config",
               "Invalid configuration line: 'sp.disabled_functions%s':"
               "'.r_param' and '.param' are mutually exclusive on line %zu.",
               line, sp_line_no);
    return -1;
  } else if (df->r_ret && df->ret) {
    sp_log_err("config",
               "Invalid configuration line: 'sp.disabled_functions%s':"
               "'.r_ret' and '.ret' are mutually exclusive on line %zu.",
               line, sp_line_no);
    return -1;
  } else if ((df->r_ret || df->ret) && (df->r_param || df->param)) {
    sp_log_err("config",
               "Invalid configuration line: 'sp.disabled_functions%s':"
               "`ret` and `param` are mutually exclusive on line %zu.",
               line, sp_line_no);
    return -1;
  } else if (!(df->r_function || df->function)) {
    sp_log_err("config",
               "Invalid configuration line: 'sp.disabled_functions%s':"
               " must take a function name on line %zu.",
               line, sp_line_no);
    return -1;
  } else if (!(df->allow ^ df->drop)) {
    sp_log_err("config",
               "Invalid configuration line: 'sp.disabled_functions%s': The "
               "rule must either be a `drop` or and `allow` one on line %zu.",
               line, sp_line_no);
    return -1;
  }

  if (df->function) {
    df->functions_list = parse_functions_list(df->function);
  }

  if (df->param && strchr(df->param, '[')) {  // assume that this is an array
    df->param_array_keys = sp_new_list();
    if (0 != array_to_list(&df->param, &df->param_array_keys)) {
      pefree(df->param_array_keys, 1);
      return -1;
    }
    df->param_is_array = 1;
  }
  
  if (df->var && strchr(df->var, '[')) {  // assume that this is an array
    df->var_array_keys = sp_new_list();
    if (0 != array_to_list(&df->var, &df->var_array_keys)) {
      pefree(df->var_array_keys, 1);
      return -1;
    }
    df->var_is_array = 1;
  }

  bool match = false;
  const char *key[4] = {"include", "include_once", "require", "require_once"};
  for (size_t i = 0; i < 4; i++) {
    if (df->r_function && true == is_regexp_matching(df->r_function, key[i])) {
      match = true;
      break;
    } else if (df->function && 0 == strcmp(df->function, key[i])) {
      match = true;
      break;
    }
  }
  if (true == match && df->regexp) {
    sp_list_insert(
        SNUFFLEUPAGUS_G(config).config_regexp_inclusion->regexp_inclusion,
        df->regexp);
  } else if (df->ret || df->r_ret || df->ret_type) {
    sp_list_insert(
        SNUFFLEUPAGUS_G(config).config_disabled_functions_ret->disabled_functions,
        df);
  } else {
    sp_list_insert(
        SNUFFLEUPAGUS_G(config).config_disabled_functions->disabled_functions,
        df);
  }
  return ret;
}

int parse_upload_validation(char *line) {
  bool disable = false, enable = false;
  sp_config_functions sp_config_funcs_upload_validation[] = {
      {parse_str, SP_TOKEN_UPLOAD_SCRIPT,
       &(SNUFFLEUPAGUS_G(config).config_upload_validation->script)},
      {parse_empty, SP_TOKEN_SIMULATION,
       &(SNUFFLEUPAGUS_G(config).config_upload_validation->simulation)},
      {parse_empty, SP_TOKEN_ENABLE, &(enable)},
      {parse_empty, SP_TOKEN_DISABLE, &(disable)},
      {0}};

  int ret = parse_keywords(sp_config_funcs_upload_validation, line);

  if (0 != ret) {
    return ret;
  }

  if (!(enable ^ disable)) {
    sp_log_err("config", "A rule can't be enabled and disabled on line %zu.", sp_line_no);
    return -1;
  } 
  SNUFFLEUPAGUS_G(config).config_upload_validation->enable = enable;

  char const *script = SNUFFLEUPAGUS_G(config).config_upload_validation->script;

  if (!script) {
    sp_log_err("config", "The `script` directive is mandatory in '%s' on line %zu.",
      line, sp_line_no);
    return -1;
  } else if (-1 == access(script, F_OK)) {
    sp_log_err("config", "The `script` (%s) doesn't exist on line %zu.", script, sp_line_no);
    return -1;
  } else if (-1 == access(script, X_OK)) {
    sp_log_err("config", "The `script` (%s) isn't executable on line %zu.", script, sp_line_no);
    return -1;
  }
  
  return ret;
}
