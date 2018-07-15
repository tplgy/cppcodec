# TODO

cppcodec is in pretty good shape already.
Here are a number of things I'd like to do still:

* Stuff in the GitHub issues list.

* Implement place-based single number codecs (as opposed to stream codecs) that
  view the entire input string as a single number and therefore zero-extend
  *to the left* to the next bit multiple (e.g. n*5 for base32, n*4 for hex).
  We want this to implement odd hex decoding (e.g. 0xF rather than 0x0F) and
  the other interpretation of Crockford base32. No use case seems to exist for
  base64 because it's thankfully specified well enough to always encode
  octet streams, not numbers.
  * API idea: Specialize the encode(Result&, const T&) overload to for T = number
    and use that to accept numbers without changing the interface. Not sure if
    it's a good idea to switch to a place-based single number codec based on T.
  * API idea: Instead of trying to fit both into the same interface, make a
    separate interface just for numbers. No binary arrays, templates only on
    the encoded side.
  * Naming: Current plan is to name place-based single number variants as the
    original variant name plus `_num` appended. Examples: `base32_crockford_num`,
    `hex_upper_num`, `hex_lower_num`.
  * Since we don't know the total number of symbols (could be ignored characters?)
    we might have to **(a)** assume there is no whitespace (fail) or
    **(b)** walk the source data twice, the first time for counting symbols and
    the second time for putting them into their right spot in the byte.

* Investigate binary size considerations. See how well inline deduplication
  works in popular linkers. I've had good experiences with boost::asio but
  I don't know if those can translate to a codec library.

* See if binary size would be saved if std::vector<[unsigned] char> could
  return a temporary raw_result_buffer instead of being passed down as itself,
  for use cases where both std::vector and raw pointer calls are in use.

* More codec variants:
  * binary - useful for debugging
  * octal
  * z-base32 might be interesting (and has some funky marginal-space-savings
    options if your input length isn't octets), but doesn't appear any more
    popular than Crockford base32. Pretty far down on the list.
  * base64 variants from PEM (RFC 1421), MIME (RFC 2045) and UTF-7 (RFC 2152)
    since they're popular and less strict than RFC 4648. Requires more
    sophisticated generation of whitespace and ideally also checks whether
    the whitespace is correctly located in the input string.
  * Proquints? I'm not quite sure about how useful those are in real life.

* Checksums: Crockford base32 and RFC 6920 unpadded base64url define optional
  checksums, OpenPGP base64 has a mandatory one. Supporting these would mean
  a change to the API, potentially together with other options (but not
  necessarily so).

* User options: I'm not too big on accepting invalid/non-conformant input,
  but maybe somebody has a valid use case where they need to be more lenient
  than one of the standards where the solution shouldn't be a new codec variant
  but instead options for an existing variant? I'm not convinced that's a good
  idea right now, but if you think it is then please make a point.
  * We'll probably want some kind of unspecified whitespace acceptance for hex,
    maybe there should just be a template version of cppcodec::hex for that
    with ignored characters as the template.
  * Crockford base32 allows hyphens as visual delimiter (ignored when decoding)
    but doesn't specify
