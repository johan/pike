#pike 7.7

class Simple
{
  inherit Parser.XML.Simple;

  static void create()
  {
    compat_allow_errors ("7.2");
    // Got no inherited create.
  }
}

class Validating
{
  inherit Parser.XML.Validating;

  static void create()
  {
    compat_allow_errors ("7.2");
    // Got no inherited create.
  }
}
