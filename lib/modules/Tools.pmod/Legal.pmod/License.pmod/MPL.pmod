#pike __REAL_VERSION__

// $Id$

private constant text = #string "mpl.txt";

string get_text() {
  return text;
}

string get_xml() {
  return "<pre>\n" + get_text() + "</pre>\n";
}
