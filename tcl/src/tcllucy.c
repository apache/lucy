#include "tcllucy.h"

void delete_termqueryCmd(ClientData clientData){
  //  lucy_termquery *tq;
  //  tq=(lucy_termquery *)clientData;
  //  ckfree(tq);
}

int x_termqueryObjCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]){
  /* a generic termquery instance. */

  if(1==objc){
    Tcl_WrongNumArgs(interp, 1, objv, "subcommand ?arg?");
    return TCL_ERROR;
  }

  int subcmd_idx, rc, field_len;
  char *field;
  Tcl_Obj *retObj;
  lucy_termquery *tq;
  tq=(lucy_termquery *)clientData;

  enum {
    TL_SETFIELD,
    TL_GETFIELD
  };

  static const char *termquery_subcmd[]={
    "setfield",
    "getfield",
    NULL
  };

  if(TCL_ERROR==Tcl_GetIndexFromObj(interp, objv[1], termquery_subcmd, "subcommand", 0, &subcmd_idx))
    return TCL_ERROR;

  switch(subcmd_idx){
  case TL_SETFIELD:
    if(3!=objc){
      Tcl_WrongNumArgs(interp, 2, objv, "value");
      return TCL_ERROR;
    }else{
      if(tq->field)
	ckfree(tq->field);
      field=Tcl_GetStringFromObj(objv[2], &field_len);
      // lucy_load_field(tq, field, field_len); assuming copies input string...
      tq->field=ckalloc(sizeof(char)*(field_len+1));
      strcpy(tq->field, field);
      Tcl_SetObjResult(interp, Tcl_NewStringObj(field, field_len));
      rc=TCL_OK;
    }
    break;
  case TL_GETFIELD:
    if(2!=objc){
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      rc=TCL_ERROR;
    }else{
      //    retObj=Tcl_NewStringObj(lucy_get_queryfield(tq), get_queryfield_len(tq));
      if(tq->field){
	retObj=Tcl_NewStringObj(tq->field, -1);
	Tcl_SetObjResult(interp, retObj);
	rc=TCL_OK;
      }else{
	retObj=Tcl_NewStringObj("field unset", 11);
	Tcl_SetObjResult(interp, retObj);
	rc=TCL_ERROR;
      }
    }
    break;
  default:
    assert("subcmd not accounted-for in switch()" && 0);
  }

  return rc;
}

int new_termqueryObjCmd(ClientData *clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]){
  /* create cmd, instantiate state */
  /* usage: termquery name */
  if(2!=objc){
    Tcl_WrongNumArgs(interp, 1, objv, "name");
    return TCL_ERROR;
  }

  lucy_termquery *tq;
  tq=ckalloc(sizeof(lucy_termquery));
  tq->field=NULL;		/* not yet set... */

  /* should check if clobbering existing cmd, but we're not */
  Tcl_CreateObjCommand(interp, Tcl_GetString(objv[1]), x_termqueryObjCmd, (ClientData *)tq, delete_termqueryCmd);
  Tcl_SetObjResult(interp, Tcl_DuplicateObj(objv[1])); /* return newly created cmdname as result */
  return TCL_OK;
}

int Tcllucy_Init(Tcl_Interp *interp){
  if (NULL == Tcl_InitStubs(interp, "8.4",0)){
    Tcl_SetObjResult(interp, Tcl_NewStringObj("Error in \"Tcl_InitStubs\"", 24));
    return TCL_ERROR;
  }
  
  Tcl_Namespace *namespace;
  namespace = Tcl_CreateNamespace(interp, "lucy", NULL, NULL);
  Tcl_CreateObjCommand (interp, "lucy::termquery", (Tcl_ObjCmdProc *)new_termqueryObjCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
  Tcl_Export(interp, namespace, "*", 0);
  Tcl_PkgProvide(interp, "lucy", "0.0.1");
  return TCL_OK;
}
