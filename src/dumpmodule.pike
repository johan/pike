#!/usr/local/bin/pike

program p;

#define error(X) throw( ({ (X), backtrace()[0..sizeof(backtrace())-2] }) )
class Codec
{
  string nameof(mixed x)
  {
//    werror("%O\n",x);
    if(p!=x)
      if(mixed tmp=search(all_constants(),x))
	return "efun:"+tmp;

    switch(sprintf("%t",x))
    {
      case "program":
	if(p!=x)
	{
	  if(mixed tmp=search(master()->programs,x))
	    return tmp;
#if 0
	  if(mixed tmp=search(values(_static_modules), x)!=-1)
	  {
	    return "_static_modules."+(indices(_static_modules)[tmp]);
	  }
#endif
	}
	break;

      case "object":
	if(p!=x)
	  if(mixed tmp=search(master()->objects,x))
	    if(tmp=search(master()->programs,tmp))
	      return tmp;
	break;
    }
    return ([])[0];
  } 

  function functionof(string x)
  {
    if(sscanf(x,"efun:%s",x))
      return all_constants()[x];

    werror("Failed to decode %s\n",x);
    return 0;
  }


  object objectof(string x)
  {
    if(sscanf(x,"efun:%s",x))
      return all_constants()[x];

    if(object tmp=(object)x) return tmp;
    werror("Failed to decode %s\n",x);
    return 0;
    
  }

  program programof(string x)
  {
    if(sscanf(x,"efun:%s",x))
      return all_constants()[x];

    if(sscanf(x,"_static_modules.%s",x))
    {
      return (program)_static_modules[x];
    }

    if(program tmp=(program)x) return tmp;
    werror("Failed to decode %s\n",x);
    return 0;
  }

  mixed encode_object(object x)
  {
    error("Cannot encode objects yet.\n");
  }


  mixed decode_object(object x)
  {
    error("Cannot encode objects yet.\n");
  }
}

int main(int argc, string *argv)
{
  foreach(argv[1..],string file)
    {
      werror(file +": ");
      mixed err=catch {
	if(mixed s=file_stat(file))
	{
	  if(s[1]<=0)
	  {
	    werror("is a directory or special file.\n");
	    break;
	  }
	}else{
	  werror("does not exist.\n");
	  break;
	}
	if(programp(p=compile_file(file)))
	{
	  string s=encode_value(p, Codec());
	  p=decode_value(s,Codec());
	  if(programp(p))
	  {
	    Stdio.File(file + ".o","wct")->write(s);
	    werror("dumped.\n");
	  }else{
	    werror("Decode failed.\n");
	  }
	}else{
	  werror("Compilation failed.\n");
	}
      };
      if(err)
      {
#ifdef ERRORS
	err[0]="While dumping "+file+": "+err[0];
	werror(master()->describe_backtrace(err));
#else
	werror(err[0]);
#endif
      }
    }
}