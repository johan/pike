#include "types.h"
#if __VERSION__ >= 0.6
import ".";
#endif /* __VERSION__ >= 0.6 */
import Sgml;

/*
 * My dilemma:
 *
 * WMML is not a very good 'in between'-format because it
 * does not allow to send structured mappings to the output
 * generators. Because of this the output generator has to
 * do extensive parsing to find the proper structure elements
 * *OR* it becomes dependant on the order that the data is
 * delivered to it, which is *not* good.
 *
 * Example:
 * <function><man_title>bla bla</man_title><man_desc>WMML DATA</man_desc></man_desc>
 * Would be easier for the output generator to parse as:
 * <function (["man_title":({"bla bla"}), "man_desc":WMML DATA ])>
 *
 * Possible solution one:
 * Create a link in the properties mapping to the WMML segment in question,
 * the result of the above would be:
 *   X=Tag("man_title",([]),0,({"bla bla"}));
 *   Y=Tag("man_desc",([]),0,WMML DATA);
 *   Tag("function",(["<man_title>":X,"man_desc":Y]),0,({X,Y}));
 * advantages of this method:
 *   backwards compatible (with most things)
 * disadvantages of this method:
 *
 * Solution two:
 *   Same as above, but using another field in the Tag class.
 *
 * Solution three: (implemented)
 *   Add a function to the Tag class which can access the contained
 *   tags as a mapping.
 *
 */

SGML low_make_concrete_wmml(SGML data);

#define TNAME(X) (objectp(X) ? X->tag : "")
#define PROCESS(X,T) foreach(X,TAG T) switch(objectp(T) ? T->tag : "")

string expand_macro_string(string s, TAG tag)
{
  array(string) data=s/"$";
  for(int e=1;e<sizeof(data);e+=2)
  {
    if(data[e]=="")
    {
      data[e]="$";
    }else{
      data[e]=tag->params[data[e]];
    }
  }
  return data*"";
}

SGML expand_macro(SGML macro, TAG tag)
{
  SGML ret=({});
  
  foreach(macro, TAG t)
    {
      if(objectp(t))
      {
	if(sscanf(t->tag,"%s:%s",string tagtype, string what))
	{
	  switch(tagtype)
	  {
	    case "param": ret+=({ tag->params[what] }); continue;
	    case "taglist": ret+=tag->find(what); continue;
	    case "tag": ret+=tag->find(what)->data; continue;
	  }
	}else{
	  switch(t->tag)
	  {
	    case "content": ret+=tag->data; continue;
	  }
	}

	ret+=({
	  Tag(t->tag,
	      map(t->params, expand_macro_string, tag),
	      t->pos,
	      expand_macro(t->data, tag),
	      t->file)
	});
      }else{
	ret+=({ expand_macro_string(t, tag) });
      }
    }
  return ret;
}

mapping(string:SGML) macros=([]);

SGML expand_macros(SGML x)
{
  if(!x) return 0;

  SGML ret=({});

  foreach(x, TAG t)
    {
      if(objectp(t))
      {
	switch(t->tag)
	{
	  case "include":
	  {
	    string filename=t->params->file;
	    werror("Including %s...\n",filename);
	    string file=Stdio.read_file(filename);
	    if (file)
	    {
	      ret+=expand_macros(group(lex(file,filename)));
	    } else {
	      werror(sprintf("File %O not found specified in tag %O near %s\n",
			     filename, t->tag, t->location()));
	    }
	    continue;
	  }
	  
	  case "defmacro":
	    macros[t->name]=t->data;
	    continue;

	  default:
	    if(SGML macro=macros[t->name])
	    {
	      m_delete(macros, t->name); /* Avoid recursive expansion */
	      ret+=expand_macros( expand_macro(macro, t) );
	      macros[t->name]=macros;
	      continue;
	    }
	}
	ret+=({
	  Tag(t->tag,
	      t->params,
	      t->pos,
	      expand_macros(t->data),
	      t->file)
	});
	continue;
      }
      ret+=({t});
    }
  return ret;
}


class Trace
{
  Trace prev;
  Tag tag;

  void create(Trace p, Tag t)
  {
    prev=p;
    tag=t;
  }

  string dump()
  {
    return 
      (tag ? "  In tag "+(tag->tag=="anchor"?tag->tag+" (name="+tag->params->name+")":tag->tag)+" near "+tag->location()+"\n" : "")+
      (prev?prev->dump():"");
  }
}



// This function verifies SGML data
// FIXME: check for superflous tags in <table>
// FIXME: put a cap on the <section> depth
static private int verify_any(SGML data,
			      Trace in,
			      string input,
			      string input_name)
{
  int i=1;
  foreach(data,mixed x)
  {
    if(objectp(x))
    {
      if(x->file == input_name)
      {
	if(input[x->pos-1]!='<')
	{
	  werror("Location out of sync in tag "+x->tag+" near "+x->location()+"\n");
	  werror(in->dump());
	}
      }
     
      if(strlen(x->tag) && x->tag[0]=='/')
      {
	werror("Unmatched "+x->tag+" near "+x->location()+"\n");
	werror(in->dump());
	i=0;
	continue;
      }
      switch(x->tag)
      {
	 default:
	    werror("Unknown tag "+x->tag+" near "+x->location()+".\n");
	    werror(in->dump());
	    i=0;
	    break;

	 case "font":
	 case "firstpage":
	 case "preface":
	 case "introduction":

	 case "chapter":
	 case "appendix":
	 case "i":
	 case "b":
	 case "a":
	 case "anchor":
	 case "tt":
	 case "pre":
	 case "tr":
	 case "td":
	 case "table":
	 case "h1":
	 case "h2":
	 case "h3":
	 case "dl":
	 case "ul":
	 case "section":
	 case "center":
	 case "ol":
	 case "encaps":
	 case "th":
	 case "illustration":
	 case "link":

         case "sup":
         case "sub":

	 case "add_appendix":

	 case "exercises":
	 case "exercise":

	 case "man_nb":
	 case "module":
	 case "class":
	 case "method":
	 case "variable":
	 case "function":
	 case "constant":
	 case "man_description":
	 case "man_see":
	 case "man_syntax":
	 case "man_bugs":
	 case "man_example":
	 case "man_title":
	 case "man_arguments":
	 case "man_returns":
	 case "man_note":

	 case "ex_identifier":
	 case "ex_keyword":
	 case "ex_string":
	 case "ex_comment":
	 case "ex_meta":
	 case "example":

	 case "data_description":
	 case "elem": // in data_description

	 case "execute":
	 
	 case "aargdesc": // in man_arguments
	 case "aarg":	  // in man_arguments
	 case "adesc":    // in man_arguments
	    if(!x->data)
	    {
	       werror("Tag "+x->tag+" not closed near "+x->location()+".\n");
	       werror(in->dump());
	       i=0;
	    }

	    break;

	 case "image":
	   if(!x->params->src &&
	      !x->params->gif &&
	      !x->params->xfig &&
	      !x->params->jpeg &&
	      !x->params->eps)
	   {
	     werror("Image without source near "+x->location()+"\n");
	     werror(in->dump());
	     i=0;
	   }

	 case "insert_added_appendices":
	 case "ex_indent":
	 case "ex_br":
//	 case "include":
	 case "dt":
	 case "dd":
	 case "li":
	 case "ref":
	 case "br":
         case "hr":
	 case "img":
	      
	 case "table-of-contents":
	 case "index":
	    if(x->data)
	    {
	       werror("Tag "+x->tag+" should not be closed near "+x->location()+"\n");
	       werror(in->dump());
	       i=0;
	    }
	 case "p":
	 case "wbr":
      }

      if(x->data)
	if(!verify_any(x->data,
		       Trace(in,x),
		       input,
		       input_name))
	  i=0;
    }
  }
  return i;
}

int verify(SGML data, string input, string input_name)
{
  return verify_any(data,Trace(0,0), input, input_name);
}


// Generate an index
// Might not be required for all output formats
//
INDEX_DATA collect_index(SGML data, void|INDEX_DATA index,void|mapping taken)
{
  if(!index) index=([]);
  if(!taken) taken=([]);

  foreach(data,TAG data)
  {
    if(objectp(data))
    {
      if(data->tag == "anchor"
#if 0 // Let's try to add these to the 
	  && !data->params->type
#endif
	)
      {
	if(string real_name=data->params->name)
	{
	  string new_name=real_name;

	  if(!strlen(new_name))
	  {
	    werror("Empty link name in <anchor> at %s\n",data->location());
	    continue;
	  }
	      
	  
	  if(taken[new_name])
	  {
	    int n=2;
//	    werror("Warning, duplicate "+real_name+" near "+data->location()+".\n");
	    while(taken[new_name+"_"+n]) n++;
	    new_name+="_"+n;
	  }
	  taken[new_name]++;
	  data->params->name=new_name;
	  
	  if(index[real_name])
	  {
	    index[real_name]+=({new_name});
	  }else{
	    index[real_name]=({new_name});
	  }
	}else{
	  werror("<anchor> without name near %s\n",data->location());
	}
      }
      if(data->data)
	collect_index(data->data,index,taken);
    }
  }
  return index;
}

INDEX newind;
INDEX_DATA index;

static private void build_index(string from, string fullname)
{
  mapping m=newind;

  if(string *to=index[fullname])
  {
    foreach(from/".",string tmp)
      {
	if(mapping m2=m[tmp])
	  m=m2;
	else
	  m=m[tmp]=([]);
      }
    if(!m[0]) m[0]=([]);
    m[0][fullname]=to;
  }
}

INDEX group_index(INDEX_DATA i)
{
  newind=([]);
  index=i;
  foreach(indices(index),string real_name)
  {
    string *to=index[real_name];
    build_index(real_name,real_name);
    string *from=real_name/".";
    for(int e=1;e<sizeof(from);e++)
      build_index(from[e], from[..e]*".");
  }
  index=0;
  return newind;
}

INDEX group_index_by_character(INDEX i)
{
  mapping m=([]);
  foreach(indices(i),string key)
    {
      int c;
      string char;
      if(sscanf(lower_case(Html.unquote_param(key)),"%*[_ ]%c",c)==2)
      {
	char=upper_case(sprintf("%c",c));
      }else{
	char="";
      }

//      werror(char +" : "+key+"\n");
      if(!m[char]) m[char]=([]);
      m[char][key]=i[key];
    }
  return m;
}


// Rerserver dowrds in Pike
string * reserved_pike =
({
  "array","break","case","catch","continue","default","do","else","float",
  "for","foreach","function","gauge","if","inherit","inline","int","lambda",
  "mapping","mixed","multiset","nomask","object","predef","private","program",
  "protected","public","return","sscanf","static","string","switch","typeof",
  "varargs","void","while"
});


// Reserved words in C
string *reserved_c =
({
  "break","case","continue","default","do","else","float","double",
  "for","if","int","char","short","unsigned","long",
  "public","return","static","switch",
  "void","while"
});


// Parce C/C++/Pike/Java and highlight
//
object(Tag) parse_pike_code(string x,
	 int pos,
	 mapping(string:string) reserved_type)
{
  int p,e;
  int tabindented=-1;
  SGML ret=({""});

  while(e<strlen(x) && (x[e]=='\t' || x[e]=='\n' || x[e]==' ')) e++;
  while(e && (e>=strlen(x) || x[e]!='\n')) e--;

  for(;e<strlen(x);e++)
  {
    switch(x[e])
    {
    case '_':
    case 'a'..'z':
    case 'A'..'Z':
    {
      p=e;
      
      while(1)
      {
	switch(x[++e])
	{
	case '_':
	case 'a'..'z':
	case 'A'..'Z':
	case '0'..'9':
	  continue;
	}
	break;
      }
      
      string id=x[p..--e];
      ret+=({ Tag(reserved_type[id]||"ex_identifier",([]), pos+e, ({ id }) ) });
      break;
    }
    
    
    case '\'':
      p=e;
      while(x[++e]!='\'')
	if(x[e]=='\\')
	  e++;
      ret+=({ Tag("ex_string",([]), pos+e, ({ x[p..e]}) ) }); 
      break;
      
    case '"':
      p=e;
      while(x[++e]!='"')
	if(x[e]=='\\')
	  e++;
      
      ret+=({ Tag("ex_string",([]), pos+e, ({ x[p..e] }) ) });
      break;

    case '\n':
      if(tabindented!=-1)
	ret+=({ Tag("ex_br", ([]), pos+e) });

      if(tabindented)
      {
	for(int y=1;y<9;y++)
	{
	  switch(x[e+y..e+y])
	  {
	  case " ": continue;
	  case "\t": e+=y;
	    tabindented=1;
	  default:
	  }
	  break;
	}
	if(tabindented==-1) tabindented=0;
      }

      while(x[e+1..e+1]=="\t")
      {
	ret+=({
	  Tag("ex_indent", ([]), pos+e),
	  Tag("ex_indent", ([]), pos+e),
	  Tag("ex_indent", ([]), pos+e),
	  Tag("ex_indent", ([]), pos+e),
	    });
	e++;
      }

      while(x[e+1..e+2]=="  ")
      {
	ret+=({ Tag("ex_indent", ([]), pos+e) });
	e+=2;
      }
      break;

    case '/':
      if(x[e+1..e+1]=="/")
      {
	p=e++;
	while(x[e]!='\n') e++;
	e--;
	ret+=({ Tag("ex_comment",([]), pos+e, ({ x[p..e]}) ) }); 
	break;
      }
	
      if(x[e+1..e+1]=="*")
      {
	p=e++;
	while(!(< "", "*/" >)[x[e..e+1]])
	  e++;
	e++;
	ret+=({ Tag("ex_comment",([]), pos+e, ({ x[p..e]}) ) }); 
	break;
      }
      
    default:
      ret[-1]+=x[e..e];
      continue;
    }
    ret+=({ "" });
  }

  return Tag("example",([]),pos,ret);
}

// All data that will be sent to the
// output generator.
class Wmml
{
  // Header data
  SGML metadata;

  // Table of contents
  TOC toc;

  // Index
  INDEX_DATA index_data;

  // Concrete WMML data
  SGML data;

  // Link name to tag mapping
  mapping(string:TAG) links;
};

// Enumerators are used to give numbers to chapters,
// sections and appendixes
class Enumerator
{
  string *base;

  object push() { base+=({"1"}); return this_object(); }
  object pop()  { base=base[..sizeof(base)-2]; return this_object();  }
  object inc() 
  {
    switch(base[-1][0])
    {
      case '0'..'9':
	base[-1]=(string)( 1+ (int) base[-1]);
	break;

      default:
	base[-1]=sprintf("%c",base[-1][0]+1);
    }

    return this_object();
  }

  string query()
  {
    return base*".";
  }

  void create(mixed b)
  {
    if(objectp(b)) b=b->base;
    if(!arrayp(b)) b=({b});
    base=b;
  }
};

class Stacker
{
  inherit Enumerator;

  object push(string what) { base+=({what}); return this_object(); }
  object pop()  { base=base[..sizeof(base)-2]; return this_object();  }

  void create(void|mixed b)
  {
    ::create(b||({}));
  }
};

class TocBuilder
{
  import Sgml;

  TOC *stack=({ ({}) });

  void push() { stack+=({ ({}) });  }

  void pop(TAG t)
  {
    stack[-2]+=({
      Tag(t->tag+"_toc",
	  t->params+([]),
	  t->pos,
	  stack[-1],
	  t->file)
    });
    stack=stack[..sizeof(stack)-2];
  }

  TOC query() { return stack[0]; }
};


SGML metadata;
object(Enumerator) currentE;
object(Enumerator) appendixE;
object(Enumerator) chapterE;
object(Stacker) classbase;
object(TocBuilder) toker;

array add_appendices=({});

string name_to_link(string x)
{
   return replace(x,({"->","-&gt;", "#"}),({".",".",""}));
}

string name_to_link_exp(string x)
{
   x=name_to_link(x);
   if (search(x,".")==-1) 
      x=classbase->query()+"."+x;
   return x;
}

SGML fix_anchors(TAG t)
{
  TAG ret=t;
  if(t->params->number)
  {
    ret=Tag("anchor",
	    (["name":t->params->number,"type":t->tag]),
	    t->pos,
	    ({ret}),
	    t->file);
  }

  if(t->params->name)
  {
    ret=Tag("anchor",
	    (["name":t->params->name,"type":t->tag]),
	    t->pos,
	    ({ret}),
	    t->file);
  }
  return ({ret});
}

SGML fix_section(TAG t, object(Enumerator) e)
{
  object(Enumerator) save=currentE;
  string num=e->query();
  e->push();
  currentE=e;

  toker->push();

  TAG ret=Tag(t->tag,
	      t->params+(["number":num]),
	      t->pos,
	      low_make_concrete_wmml(t->data),
	      t->file);

  toker->pop(ret);

  currentE->pop();
  currentE=save;

  return fix_anchors(ret);
}

array(array(string)) fix_names(Tag tag)
{
  array anchors=({}),fullnames=({});
  string last;
  
  foreach ( replace(tag->params->name,
			    ({"&gt;","&lt;"}),({">","<"}))/",",
		   string name)
    {
      string fullname;
      if (name!="" && name[0]=='.')
	fullname=last+name[1..];
      else if (name[0..strlen(classbase->query())-1]==
	       classbase->query() ||
	       tag->params->fullpath)
	fullname=name;
      else switch(tag->tag)
      {
	case "class":
	case "module":
	case "method":
	case "variable":
	  fullname=classbase->query()+"."+name; // -> ?
	  break;
	case "function":
	case "constant":
	  fullname=classbase->query()+"."+name;
	  break;
      }
      anchors+=({name_to_link(fullname)});
      fullnames+=({fullname});
      last=reverse(array_sscanf(reverse(fullname),"%*[^.>]%s")[0]);
    }
  return ({anchors, fullnames});
}

SGML mkheader(TAG tag, array(string) fullnames, array(string) anchors)
{
#if 1
  array res=
    ({Tag("man_title",(["title":upper_case(tag->tag)]),
	  tag->pos,
	  Array.map(
	    fullnames,
	    lambda(string name,int pos)
	    { return ({Tag("tt",([]),pos,({name}))}); },
	    tag->pos)
	  *({ ",", Tag("br") })
	  +({ (tag->params->title
	       ?" - "+tag->params->title
	       :"")}))
    }) + tag->data;
#else

  /* This would probably look nicer, but it needs to be fixed... */
  SGML res= ({
    Tag("table",(["border":"0","wmml-type":tag->tag]),tag->pos,({
      Tag("tr",0,tag->pos,({
	Tag("td",0,tag->pos,
	  Array.map(
	    fullnames,
	    lambda(string name,int pos)
	    {
	      return ({Tag("h2",([]),tag->pos,({name}))});
	    })
	    *({ Tag("br") })
	  ),
	Tag("td",0,tag->pos,
	    ({ (tag->params->title ?" - "+tag->params->title :"")})
	  ),
      }))
    }) ) }) + tag->data;
#endif
  
  res=
    ({Tag(tag->tag,(["name":fullnames*", ",
		    "title":tag->params->title]),tag->pos,
	  low_make_concrete_wmml(res))});
  
  // FIXME:
  // The replace "->" to "."
  // in here is kludgy and might not be correct,
  // check for accuracy....
  // -Hubbe
  foreach (anchors,string anchor)
    res=({Tag("anchor",(["name":replace(anchor,"->",".")]),
	      tag->pos,res)});
  
  return res;
}

SGML fix_class(TAG tag, string name)
{
  array(string) anchors, fullnames;
  [anchors, fullnames]=fix_names(tag);

  classbase->push(name);

#if 0
  /* I wish I remember what this magic is for -Hubbe */
  TAG foo=Tag(tag->tag,
	      tag->params+(["name":classbase->query()]),
	      tag->pos,
	      tag->data=low_make_concrete_wmml(tag->data),
	      tag->file);

  SGML res=
  ({
    Tag("anchor",
	(["name":classbase->query(),"type":tag->tag]),
	tag->pos,
	({tag}),
	tag->file)
      });
#else
  SGML res=mkheader(tag, fullnames, anchors);
#endif
  classbase->pop();
  return res;
}

SGML fix_module(TAG tag, string name)
{
  array(string) anchors, fullnames;
  [anchors, fullnames]=fix_names(tag);

  classbase->push(name);

#if 1
  /* I wish I remember what this magic is for -Hubbe */
  TAG foo=Tag(tag->tag,
	      tag->params+(["name":classbase->query()]),
	      tag->pos,
	      tag->data=low_make_concrete_wmml(tag->data),
	      tag->file);

  SGML res=
  ({
    Tag("anchor",
	(["name":classbase->query(),"type":tag->tag]),
	tag->pos,
	({tag}),
	tag->file)
      });
#else
  SGML res=mkheader(tag, fullnames, anchors);
#endif
  classbase->pop();
  return res;
}


SGML low_make_concrete_wmml(SGML data)
{
  if(!data) return 0;
  SGML ret=({});
  SGML tmp=({});

#if 1
  /* This part is intended to convert <p/> to <p> ... </p>
   * Hmm, maybe we should encapsulate *all* paragraphs in <p></p> ?
   * (But how to we recognize am unmarked paragraph as opposed to
   *  an argument? Example the text within <man_see></man_see> should
   *  *NOT* be in a <p>)
   */
  PROCESS(reverse(data), ptag)
    {
      default:
	tmp+=({ptag});
	break;

      case "p":
	ret+=({
	  Tag(ptag->tag,
	      ptag->params,
	      ptag->pos,
	      reverse(tmp),
	      ptag->file)
	});
	tmp=({});
	break;

      case "h1":
      case "h2":
      case "h3":
      case "h4":
      case "h5":
      case "h6":
      case "center":
      case "dl":
      case "ul":
      case "ol":
	tmp+=({ptag});
	ret+=tmp;
	tmp=({});
    }
  ret+=tmp;
  data=reverse(ret);

  ret=({});
#endif

  foreach(data, TAG tag)
  {
    if (stringp(tag))
    {
      ret+=({tag});
    }else if (objectp(tag)) {
      switch(tag->tag)
      {
	case "dl":
	case "ul":
	case "ol":
	{
	  array tmp2=({});
	  tmp=({});
	    
	  PROCESS(reverse(tag->data), t)
	    {
	      default:
		tmp+=({t});
		break;
		
	      case "li":
	      case "dt":
	      case "dd":
		tmp2+=({
		  Tag(t->tag,
		      t->params,
		      t->pos,
		      low_make_concrete_wmml(reverse(tmp)),
		      t->file)
		});
		tmp=({});
	    }

	  if(strlen(replace(Sgml.get_text(tmp),({" ","\t","\n","\r"}),({"","","",""}))))
	  {
	    werror("WARNING, data ignored in %s near %s\n",tag->tag,tag->location());
	  }
	  ret+=({Tag(tag->tag,
		     tag->params,
		     tag->pos,
		     low_make_concrete_wmml(reverse(tmp2)),
		     tag->file)});
	  continue;
	}
	  
	case "index":
	case "table-of-contents":
	  ret+=({
	    Tag("anchor",
		(["name":tag->params->name || tag->tag]),
		tag->pos,
		({
		  Tag(tag->tag,
		      tag->params,
		      tag->pos,
		      low_make_concrete_wmml(tag->data),
		      tag->file),
		    }),
		tag->file,
		)
	      });
	  toker->push();
	  toker->pop(tag);
	  continue;


	case "head":
	  metadata+=tag->data;
	  continue;
	  
	case "chapter":
	  ret+=fix_section(tag,chapterE);
	  chapterE->inc();
	  continue;

	case "section":
	  ret+=fix_section(tag,currentE);
	  currentE->inc();
	  continue;

	case "preface":
	  ret+=fix_section(tag,Enumerator("preface"));
	  continue;

	case "introduction":
	  ret+=fix_section(tag,Enumerator("introduction"));
	  continue;

	case "appendix":
	  ret+=fix_section(tag,appendixE);
	  appendixE->inc();
	  continue;

	case "add_appendix":
	  tag->tag="appendix";
//	  add_appendices+=fix_section(tag,appendixE);
	  add_appendices+=({tag});
	  continue;

        case "insert_added_appendices":
	  ret+=low_make_concrete_wmml(add_appendices);
	  continue;
	  
	case "table":
	case "image":
	case "illustration":
	  if(tag->params->name)
	  {
	    TAG t=Tag(tag->tag,
		      tag->params,
		      tag->pos,
		      tag->data=low_make_concrete_wmml(tag->data),
		      tag->file);
	    ret+=({
	      Tag("anchor",
		  (["name":tag->params->name,"type":tag->tag]),
		  tag->pos,
		  ({ t }))
		});
	    continue;
	  }
	  break;

        case "execute":
	  ret+=low_make_concrete_wmml(execute_contents(tag));
	  continue;
	  
	case "class":
	{
	   string name=tag->params->name;
	   if (!name)
	   {
	      werror("module or class w/o name\n"+
		     tag->location()+"\n");
	      name="(unknown)";
	   }
	   sscanf(name,classbase->query()+"%*[.->]%s",name);
	   ret+=fix_class(tag, name);
	   continue;
	}

	case "module":
	{
	   string name=tag->params->name;
	   if (!name)
	   {
	      werror("module or class w/o name\n"+
		     tag->location()+"\n");
	      name="(unknown)";
	   }
	   sscanf(name,classbase->query()+"%*[.->]%s",name);
	   ret+=fix_module(tag, name);
	   continue;
	}
	
	case "man_syntax":
	case "man_example":
	case "man_nb":
	case "man_bugs":
	case "man_description":
	case "man_see":

	case "man_arguments":
	case "man_returns":
	case "man_note":
	{
	  string title=tag->tag;
	  SGML args=tag->data;
	  sscanf(title,"man_%s",title);
	  switch(title)
	  {
	    case "nb": title="nota bene"; break;
	    case "syntax":
	    case "example":
	      args=({Tag("tt",([]),tag->pos,tag->data)});
	      break;

	    case "see":
	    {
	      title="see also";
	      string *tmp2=(replace(get_text(args),
				   ({" ","\n"}),({"",""}))/",")-({""});
	      if(!sizeof(tmp2))
		continue;
		
	      array(string) tmp=({});

	      foreach(tmp2,string name)
		{
		  tmp+=({
		    Tag("link",(["to":
				 (tag->params->exp
				  ?name_to_link_exp
				  :name_to_link)(name)]),
			tag->pos,
			({
			  Tag("tt",([]),tag->pos,({name})),
			    }),tag->file),
		      ", "
			});
		}
	      
	      tmp[-1]="";
	      if(sizeof(tmp)>3) tmp[-3]=" and ";
	      
	      args=tmp;
	      break;
	    }
	    case "arguments":
	       // insert argument parsing here 
	       // format:
	       // <aargdesc><aarg>int foo</aarg><aarg>int bar</aarg>...
               //         ...<adesc>description</adesc></aargdesc>
	       // 'desc' is formatted description text.
	       // 'arg' may contain <alink to...> tags

	       args=({Tag("arguments",([]),tag->pos,tag->data)});

	       break;
	  }
	  title=upper_case(title);
	  ret+=low_make_concrete_wmml(
	    ({
	    Tag("man_title",(["title":title]),tag->pos,args),
	      }));
	  continue;
	}

	case "method":
        case "variable":
	case "function":
        case "constant":
	{
	  array(string) anchors, fullnames;
	  [anchors, fullnames]=fix_names(tag);

#if 0
	   array res=
	     ({Tag("man_title",(["title":upper_case(tag->tag)]),
		   tag->pos,
		   Array.map(
		     fullnames,
		     lambda(string name,int pos)
		     { return ({Tag("tt",([]),pos,({name}))}); },
		     tag->pos)
		   *({ ",", Tag("br") })
		   +({ (tag->params->title
			?" - "+tag->params->title
			:"")}))
	     }) + tag->data;

	   res=
	      ({Tag(tag->tag,(["name":fullnames*", ",
			       "title":tag->params->title]),tag->pos,
		    low_make_concrete_wmml(res))});

	   foreach (anchors,string anchor)
	      res=({Tag("anchor",(["name":anchor]),
			tag->pos,res)});
	   ret+=res;
#else
	   ret+=mkheader(tag, anchors, fullnames);
#endif

	   continue;
	}

	case "example":
	mapping reswords=([]);

	if(tag->params->meta)
	  foreach(tag->params->meta/",",string t)
	    reswords[t]="ex_meta";
	

	  switch(tag->params->language)
	  {
	    case "pike":
	      foreach(reserved_pike,string keyword)
		reswords[keyword]="ex_keyword";

	      ret+=({Sgml.Tag("pre",([]),tag->pos,
			      ({parse_pike_code(tag->data[0],
						tag->pos,
						reswords)}))});
	      continue;
	      
	    case "c":
	      foreach(reserved_c,string keyword)
		reswords[keyword]="ex_keyword";

	      ret+=({Sgml.Tag("pre",([]),tag->pos,
			      ({parse_pike_code(tag->data[0],
						tag->pos,
						reswords)}))});
	      continue;
	  }
      }
      ret+=({Tag(tag->tag,
		 tag->params,
		 tag->pos,
		 low_make_concrete_wmml(tag->data),
		 tag->file)});
    }
#if 0
    else if(!tag) {
      werror("Warning: NUL encountered in contents.\n");
    }
#endif /* 0 */
    else 
       throw(({"Tag or contents has illegal type: "+sprintf("%O\n",tag),
	       backtrace()}));
  }
  return ret;
}

object(Wmml) make_concrete_wmml(SGML data)
{
  classbase=Stacker();
  currentE=0;
  appendixE=Enumerator("A");
  chapterE=Enumerator("1");
  toker=TocBuilder();
  object(Wmml) ret= Wmml();
  metadata=({});
  ret->data=low_make_concrete_wmml(data);
  ret->index_data=collect_index(ret->data);
  ret->toc=toker->query();
  ret->metadata=metadata;
  collect_links(ret->data,ret->links=([]));
  ret->data=unlink_unknown_links(ret->data,ret->links);
  
  return ret;
}

// These routines are meant to create a mapping from a link name
// to the corresponding TAG.

int useful_tag(TAG t)
{
  return !stringp(t) || sscanf(t,"%*[ \n\r\t�]%*c")==2;
}

TAG get_tag(TAG t)
{
  while(1)
  {
    switch(t->tag)
    {
      default: return t;

      case "a":
      case "anchor":
    }
    if(!t->data) return t;

    SGML x=Array.filter(t->data,useful_tag);
    if(sizeof(x)==1 && objectp(t->data[0]))
      t=t->data[0];
    else
      break;
  }

  return t;
}

// must be called after Wmml has been made concrete
void collect_links(SGML data, mapping known_links)
{
  foreach(data,TAG t)
    {
      if(objectp(t))
      {
	if(t->tag == "anchor")
	{
	  if(t->params->name)
	  {
	    known_links[t->params->name]=get_tag(t);
	  }
	}

	if(t->data)
	  collect_links(t->data,known_links);
      }
    }
}

// This finds nonworking links and makes them non-links,
// this way the output controller doesn't have to worry
// about broken links.
SGML unlink_unknown_links(SGML data, mapping known_links)
{
  SGML ret=({});

  foreach(data,TAG t)
    {
      if(objectp(t))
      {
	switch(t->tag)
	{
	  case "ref":
	    if(!t->params->to)
	    {
	      werror("<ref> without to= at %s\n",t->location());
	      continue;
	    }
	    if(!known_links[t->params->to])
	    {
	      werror("<ref to=%s> broken link at %s\n",t->params->to,t->location());
	      continue;
	    }
	    break;

	  case "link":
	    if(!t->params->to)
	    {
	      werror("<ref> without to= at %s\n",t->location());
	      ret+=unlink_unknown_links(t->data || ({}),known_links);
	      continue;
	    }
	    if(!known_links[t->params->to])
	    {
	      werror("<ref to=%s> broken link at %s\n",t->params->to,t->location());
	      ret+=unlink_unknown_links(t->data || ({}),known_links);
	      continue;
	    }
	}
	
	if(t->data)
	  t->data=unlink_unknown_links(t->data,known_links);
      }
      ret+=({t});
    }
  return ret;
}

string output_format;

object execute_cache=.Cache("execute_cache");

array execute_contents(Tag tag)
{
  random_seed(0);
   string data=get_text(tag->data);

   data=replace(data,"\n<p>","\n");

   
   string key=output_format +":"+ data;
   if(execute_cache[key])
     return Sgml.decode(execute_cache[key]);

   add_constant("mktag",
		lambda(string name,void|mapping arg,void|mixed cont)
		{
		   if (arg) arg=(mapping(string:string))arg;
		   return Sgml.Tag(name,
				   arg||([]),
				   tag->pos,
				   arrayp(cont)?cont:
				   intp(cont)?0:({cont}),
				   tag->file);
		});

   array ret;

   array cerrs=({});
   master()->set_inhibit_compile_errors(
      lambda(string file,int line,string err) 
      { 
	 cerrs+=({({line,err})}); 
      });

   mixed err=catch
   {
      object po;
      po=compile_string(sprintf("#%d %O\n\n", __LINE__, __FILE__) +
			"array _res=({({})});\n"
			"void write(mixed ...args) {\n"
			"  if (search(args, 0) == -1)\n"
			"    _res[0] += args;\n"
			"  else\n"
			"    error(\"write(): NUL data\\n\");\n"
			"}\n"
			"\n"
			"void begin_tag(string name,void|mapping p) {\n"
			"  _res=({({}),({name,p})})+_res;\n"
			"}\n"
			"\n"
			"object end_tag() {\n"
			"  if (sizeof(_res)<2)\n"
			"    error(\"end_tag w/o begin_tag\\n\");\n"
			"  object t=mktag(@_res[1],_res[0]);\n"
			"  _res=_res[2..];\n"
			"  return t;\n"
			"}\n"
			"\n"
			"#1 \"inline wmml generating code\"\n"
			+data)();
      po->main();
      ret=po->_res[0];
   };
   master()->set_inhibit_compile_errors(0);
   if (sizeof(cerrs))
   {
      werror("error while compiling inline "
	     "wmml-generating script\n");
      int offs=0;
      if (tag->params->__from__) 
	 werror("from "+tag->params->__from__+"\n");

      array dat=data/"\n";
      
      int i,mx=min(max(@column(cerrs,0))+5,sizeof(dat));
      for (i=max(0,min(@column(cerrs,0))-1);i<mx; i++)
	 werror("%5d: %s\n",i+offs+1,dat[i]);

      foreach (cerrs,array z)
	 werror("-:%d:%s\n",@z);

      return ({"<!-- failed to execute wmml generator... -->"});
   }
   else if (err)
   {
      werror("error while running inline "
	     "wmml-generating script\n");
      int offs=0;
      if (tag->params->__from__) 
	 werror("from "+tag->params->__from__+"\n");

      array dat=data/"\n";
//	master()->describe_error(err);
//      werror("%O\n",err[1]);
      foreach (reverse(err[1]),array y)
	 if (y[0]=="inline wmml generating code" &&
	     y[1]<10000) 
	 {
	    int n=y[1]; 
	    int i=max(0,n-4),mx=min(sizeof(dat)-1,n+6);
	    for (;i<mx; i++)
	       werror("%5d: %s\n",i+offs+1,dat[i]);
	    break;
	 }

      werror(master()->describe_backtrace(err));
      
      return ({"<!-- failed to execute wmml generator... -->"});
    }

   verify(ret,"","generated");

   execute_cache[key]=Sgml.encode(ret);
   return ret;
}


