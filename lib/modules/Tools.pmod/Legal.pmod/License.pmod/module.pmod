#pike __REAL_VERSION__

// $Id$

//! Returns all the licenses as a string, suitable for
//! saving as a file.
string get_text() {
  array licenses = sort(indices(Tools.Legal.License)) -
  ({ "get_text", "module" });
  string ret = sprintf("%-=80s\n\n",
		       "The Pike source is distributed under " +
		       String.implode_nicely(licenses) + ". "
		       "These licenses are listed in order below.");
  foreach(licenses, string license)
    ret += "\n\f" + Tools.Legal.License[license]->get_text();

  return ret;
}
