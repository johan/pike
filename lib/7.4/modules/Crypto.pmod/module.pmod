#pike 7.6

#if constant(Nettle.crypt_md5)
//! From Pike 7.5 an in Nettle @[pw] and @[salt] are binary strings, so
//! the result is different if any of these contains @expr{"\0"@}.
//!
//! The return type is also changed from string to int|string. If
//! the second argument is a hash, @[pw] will be hashed with the
//! salt from the hash and compared with the hash.
string crypt_md5(string pw, void|string salt) {
  sscanf(pw, "%s\0", pw);
  if(salt) {
    sscanf(salt, "%s\0", salt);
    sscanf(salt, "$1$%s", salt);
  }
  return Nettle.crypt_md5(pw,salt);
}
#endif

//! @deprecated String.string2hex
constant string_to_hex = __builtin.string2hex;

//! @deprecated String.hex2string
constant hex_to_string = __builtin.hex2string;

//! des_parity is removed. Instead there is @[Crypto.DES->fix_parity],
//! but it always outputs a key sized string.
string des_parity(string s) {
  foreach(s; int pos; int v) {
    int i = v & 0xfe;
    s[pos] = i | !Int.parity(i);
  }
  return s;
}