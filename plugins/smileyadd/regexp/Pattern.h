#ifndef __PATTERN_H__
#define __PATTERN_H__

#ifdef _WIN32
  #pragma warning(disable:4786)
#endif

#include <vector>
#include <map>

#include "bkstring.h"

class Matcher;
class NFANode;
class NFAQuantifierNode;

/**
  This pattern class is very similar in functionality to Java's
  java.util.regex.Pattern class. The pattern class represents an immutable
  regular expression object. Instead of having a single object contain both the
  regular expression object and the matching object, instead the two objects are
  split apart. The {@link Matcher Matcher} class represents the maching
  object.

  The Pattern class works primarily off of "compiled" patterns. A typical
  instantiation of a regular expression looks like:

  <pre>
  Pattern * p = Pattern::compile("a*b");
  Matcher * m = p->createMatcher("aaaaaab");
  if (m->matches()) ...
  </pre>

  However, if you do not need to use a pattern more than once, it is often times
  okay to use the Pattern's static methods insteads. An example looks like this:

  <pre>
  if (Pattern::matches("a*b", "aaaab")) { ... }
  </pre>

  This class does not currently support unicode. The unicode update for this
  class is coming soon.

  This class is partially immutable. It is completely safe to call createMatcher
  concurrently in different threads, but the other functions (e.g. split) should
  not be called concurrently on the same <code>Pattern</code>.

  <table border="0" cellpadding="1" cellspacing="0">
    <tr align="left" bgcolor="#CCCCFF">
      <td>
        <b>Construct</b>
      </td>
      <td>
        <b>Matches</b>
      </th>
    </tr>
    <tr>
      <td colspan="2">
      &nbsp;
      </td>
    </tr>
    <tr>
      <td colspan="2">
        <b>Characters</b>
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x</i></code>
      </td>
      <td>
        The character <code><i>x</i></code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\\</code>
      </td>
      <td>
        The character <code>\</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\0<i>nn</i></code>
      </td>
      <td>
        The character with octal ASCII value <code><i>nn</i></code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\0<i>nnn</i></code>
      </td>
      <td>
        The character with octal ASCII value <code><i>nnn</i></code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\x<i>hh</i></code>
      </td>
      <td>
        The character with hexadecimal ASCII value <code><i>hh</i></code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\t</code>
      </td>
      <td>
        A tab character
      </td>
    </tr>
    <tr>
      <td>
        <code>\r</code>
      </td>
      <td>
        A carriage return character
      </td>
    </tr>
    <tr>
      <td>
        <code>\n</code>
      </td>
      <td>
        A new-line character
      </td>
    </tr>
    <tr>
      <td colspan="2">
        &nbsp;
      </td>
    </tr>
    <tr>
      <td>
        <b>Character Classes</b>
      </td>
    </tr>
    <tr>
      <td>
        <code>[abc]</code>
      </td>
      <td>
        Either <code>a</code>, <code>b</code>, or <code>c</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>[^abc]</code>
      </td>
      <td>
        Any character but <code>a</code>, <code>b</code>, or <code>c</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>[a-zA-Z]</code>
      </td>
      <td>
        Any character ranging from <code>a</code> thru <code>z</code>, or
        <code>A</code> thru <code>Z</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>[^a-zA-Z]</code>
      </td>
      <td>
        Any character except those ranging from <code>a</code> thru
        <code>z</code>, or <code>A</code> thru <code>Z</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>[a\-z]</code>
      </td>
      <td>
        Either <code>a</code>, <code>-</code>, or <code>z</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>[a-z[A-Z]]</code>
      </td>
      <td>
        Same as <code>[a-zA-Z]</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>[a-z&&[g-i]]</code>
      </td>
      <td>
        Any character in the intersection of <code>a-z</code> and
        <code>g-i</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>[a-z&&[^g-i]]</code>
      </td>
      <td>
        Any character in <code>a-z</code> and not in <code>g-i</code>
      </td>
    </tr>
    <tr>
      <td colspan="2">
        &nbsp;
      </td>
    </tr>
    <tr>
      <td colspan="2">
        <b>Prefefined character classes</b>
      </td>
    </tr>
    <tr>
      <td>
        <code><b>.</b></code>
      </td>
      <td>
        Any character. Multiline matching must be compiled into the pattern for
        <code><b>.</b></code> to match a <code>\r</code> or a <code>\n</code>.
        Even if multiline matching is enabled, <code><b>.</b></code> will not
        match a <code>\r\n</code>, only a <code>\r</code> or a <code>\n</code>.
      </td>
    </tr>
    <tr>
      <td>
        <code>\d</code>
      </td>
      <td>
        <code>[0-9]</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\D</code>
      </td>
      <td>
        <code>[^\d]</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\s</code>
      </td>
      <td>
        <code>[&nbsp;\t\r\n\x0B]</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\S</code>
      </td>
      <td>
        <code>[^\s]</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\w</code>
      </td>
      <td>
        <code>[a-zA-Z0-9_]</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\W</code>
      </td>
      <td>
        <code>[^\w]</code>
      </td>
    </tr>
    <tr>
      <td colspan="2">
        &nbsp;
      </td>
    </tr>
    <tr>
      <td colspan="2">
        <b>POSIX character classes
      </td>
    </tr>
    <tr>
      <td>
        <code>\p{Lower}</code>
      </td>
      <td>
        <code>[a-z]</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\p{Upper}</code>
      </td>
      <td>
        <code>[A-Z]</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\p{ASCII}</code>
      </td>
      <td>
        <code>[\x00-\x7F]</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\p{Alpha}</code>
      </td>
      <td>
        <code>[a-zA-Z]</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\p{Digit}</code>
      </td>
      <td>
        <code>[0-9]</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\p{Alnum}</code>
      </td>
      <td>
        <code>[\w&&[^_]]</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\p{Punct}</code>
      </td>
      <td>
        <code>[!"#$%&'()*+,-./:;&lt;=&gt;?@[\]^_`{|}~]</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\p{XDigit}</code>
      </td>
      <td>
        <code>[a-fA-F0-9]</code>
      </td>
    </tr>
    <tr>
      <td colspan="2">
        &nbsp;
      </td>
    </tr>
    <tr>
      <td colspan="2">
        <b>Boundary Matches</b>
      </td>
    </tr>
    <tr>
      <td>
        <code>^</code>
      </td>
      <td>
        The beginning of a line. Also matches the beginning of input.
      </td>
    </tr>
    <tr>
      <td>
        <code>$</code>
      </td>
      <td>
        The end of a line. Also matches the end of input.
      </td>
    </tr>
    <tr>
      <td>
        <code>\b</code>
      </td>
      <td>
        A word boundary
      </td>
    </tr>
    <tr>
      <td>
        <code>\B</code>
      </td>
      <td>
        A non word boundary
      </td>
    </tr>
    <tr>
      <td>
        <code>\A</code>
      </td>
      <td>
        The beginning of input
      </td>
    </tr>
    <tr>
      <td>
        <code>\G</code>
      </td>
      <td>
        The end of the previous match. Ensures that a "next" match will only
        happen if it begins with the character immediately following the end of
        the "current" match.
      </td>
    </tr>
    <tr>
      <td>
        <code>\Z</code>
      </td>
      <td>
        The end of input. Will also match if there is a single trailing
        <code>\r\n</code>, a single trailing <code>\r</code>, or a single
        trailing <code>\n</code>.
      </td>
    </tr>
    <tr>
      <td>
        <code>\z</code>
      </td>
      <td>
        The end of input
      </td>
    </tr>
    <tr>
      <td>
        &nbsp;
      </td>
    </tr>
    <tr>
      <td colspan="2">
        <b>Greedy Quantifiers</b>
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x?</i></code>
      </td>
      <td>
        <i>x</i>, either zero times or one time
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x*</i></code>
      </td>
      <td>
        <i>x</i>, zero or more times
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x+</i></code>
      </td>
      <td>
        <i>x</i>, one or more times
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x{n}</i></code>
      </td>
      <td>
        <i>x</i>, exactly n times
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x{n,}</i></code>
      </td>
      <td>
        <i>x</i>, at least <code><i>n</i></code> times
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x{,m}</i></code>
      </td>
      <td>
        <i>x</i>, at most <code><i>m</i></code> times
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x{n,m}</i></code>
      </td>
      <td>
        <i>x</i>, at least <code><i>n</i></code> times and at most
        <code><i>m</i></code> times
      </td>
    </tr>
    <tr>
      <td>
        &nbsp;
      </td>
    </tr>
    <tr>
      <td colspan="2">
        <b>Possessive Quantifiers</b>
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x?+</i></code>
      </td>
      <td>
        <i>x</i>, either zero times or one time
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x*+</i></code>
      </td>
      <td>
        <i>x</i>, zero or more times
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x++</i></code>
      </td>
      <td>
        <i>x</i>, one or more times
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x{n}+</i></code>
      </td>
      <td>
        <i>x</i>, exactly n times
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x{n,}+</i></code>
      </td>
      <td>
        <i>x</i>, at least <code><i>n</i></code> times
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x{,m}+</i></code>
      </td>
      <td>
        <i>x</i>, at most <code><i>m</i></code> times
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x{n,m}+</i></code>
      </td>
      <td>
        <i>x</i>, at least <code><i>n</i></code> times and at most
        <code><i>m</i></code> times
      </td>
    </tr>
    <tr>
      <td colspan="2">
        &nbsp;
      </td>
    </tr>
    <tr>
      <td colspan="2">
        <b>Reluctant Quantifiers</b>
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x??</i></code>
      </td>
      <td>
        <i>x</i>, either zero times or one time
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x*?</i></code>
      </td>
      <td>
        <i>x</i>, zero or more times
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x+?</i></code>
      </td>
      <td>
        <i>x</i>, one or more times
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x{n}?</i></code>
      </td>
      <td>
        <i>x</i>, exactly n times
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x{n,}?</i></code>
      </td>
      <td>
        <i>x</i>, at least <code><i>n</i></code> times
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x{,m}?</i></code>
      </td>
      <td>
        <i>x</i>, at most <code><i>m</i></code> times
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x{n,m}?</i></code>
      </td>
      <td>
        <i>x</i>, at least <code><i>n</i></code> times and at most
        <code><i>m</i></code> times
      </td>
    </tr>
    <tr>
      <td>
        &nbsp;
      </td>
    </tr>
    <tr>
      <td colspan="2">
        <b>Operators</b>
      </td>
    </tr>
    <tr>
      <td>
        <code><i>xy</i></code>
      </td>
      <td>
        <code><i>x</i></code> then <code><i>y</i></code>
      </td>
    </tr>
    <tr>
      <td>
        <code><i>x</i></code>|<code><i>y</i></code>
      </td>
      <td>
        <code><i>x</i></code> or <code><i>y</i></code>
      </td>
    </tr>
    <tr>
      <td>
        <code>(<i>x</i>)</code>
      </td>
      <td>
        <code><i>x</i></code> as a capturing group
      </td>
    </tr>
    <tr>
      <td colspan="2">
        &nbsp;
      </td>
    </tr>
    <tr>
      <td colspan="2">
        <b>Quoting</b>
      </td>
    </tr>
    <tr>
      <td>
        <code>\Q</code>
      </td>
      <td>
        Nothing, but treat every character (including \s) literally until a
        matching <code>\E</code>
      </td>
    </tr>
    <tr>
      <td>
        <code>\E</code>
      </td>
      <td>
        Nothing, but ends its matching <code>\Q</code>
      </td>
    </tr>
    <tr>
      <td>
        &nbsp;
      </td>
    </tr>
    <tr>
      <td colspan="2">
        <b>Special Constructs</b>
      </td>
    </tr>
    <tr>
      <td>
        <code>(?:<i>x</i>)</code>
      </td>
      <td>
        <code><i>x</i></code>, but not as a capturing group
      </td>
    </tr>
    <tr>
      <td>
        <code>(?=<i>x</i>)</code>
      </td>
      <td>
        <code><i>x</i></code>, via positive lookahead. This means that the
        expression will match only if it is trailed by <code><i>x</i></code>.
        It will not "eat" any of the characters matched by
        <code><i>x</i></code>.
      </td>
    </tr>
    <tr>
      <td>
        <code>(?!<i>x</i>)</code>
      </td>
      <td>
        <code><i>x</i></code>, via negative lookahead. This means that the
        expression will match only if it is not trailed by
        <code><i>x</i></code>. It will not "eat" any of the characters
        matched by <code><i>x</i></code>.
      </td>
    </tr>
    <tr>
      <td>
        <code>(?<=<i>x</i>)</code>
      </td>
      <td>
        <code><i>x</i></code>, via positive lookbehind. <code><i>x</i></code>
        cannot contain any quantifiers.
      </td>
    </tr>
    <tr>
      <td>
        <code>(?<!<i>x</i>)</code>
      </td>
      <td>
        <code><i>x</i></code>, via negative lookbehind. <code><i>x</i></code>
        cannot contain any quantifiers.
      </td>
    </tr>
    <tr>
      <td>
        <code>(?><i>x</i>)</code>
      </td>
      <td>
        <code><i>x</i>{1}+</code>
      </td>
    </tr>
    <tr>
      <td colspan="2">
        &nbsp;
      </td>
    </tr>
    <tr>
      <td colspan="2">
        <b>Registered Expression Matching</b>
      </td>
    </tr>
    <tr>
      <td>
        <code>{<i>x</i>}</code>
      </td>
      <td>
        The registered pattern <code><i>x</i></code>
      </td>
    </tr>
  </table>

  <hr>

   <i>Begin Text Extracted And Modified From java.util.regex.Pattern documentation</i>

   <h4> Backslashes, escapes, and quoting </h4>

   <p> The backslash character (<tt>'\'</tt>) serves to introduce escaped
   constructs, as defined in the table above, as well as to quote characters
   that otherwise would be interpreted as unescaped constructs.  Thus the
   expression <tt>\\</tt> matches a single backslash and <tt>\{</tt> matches a
   left brace.

   <p> It is an error to use a backslash prior to any alphabetic character that
   does not denote an escaped construct; these are reserved for future
   extensions to the regular-expression language.  A backslash may be used
   prior to a non-alphabetic character regardless of whether that character is
   part of an unescaped construct.

   <p>It is necessary to double backslashes in string literals that represent
   regular expressions to protect them from interpretation by a compiler.  The
   string literal <tt>"&#92;b"</tt>, for example, matches a single backspace
   character when interpreted as a regular expression, while
   <tt>"&#92;&#92;b"</tt> matches a word boundary.  The string litera
    <tt>"&#92;(hello&#92;)"</tt> is illegal and leads to a compile-time error;
    in order to match the string <tt>(hello)</tt> the string literal
    <tt>"&#92;&#92;(hello&#92;&#92;)"</tt> must be used.

   <h4> Character Classes </h4>

      <p> Character classes may appear within other character classes, and
      may be composed by the union operator (implicit) and the intersection
      operator (<tt>&amp;&amp;</tt>).
      The union operator denotes a class that contains every character that is
      in at least one of its operand classes.  The intersection operator
      denotes a class that contains every character that is in both of its
      operand classes.

      <p> The precedence of character-class operators is as follows, from
      highest to lowest:

      <blockquote><table border="0" cellpadding="1" cellspacing="0"
                   summary="Precedence of character class operators.">

        <tr><th>1&nbsp;&nbsp;&nbsp;&nbsp;</th>
            <td>Literal escape&nbsp;&nbsp;&nbsp;&nbsp;</td>
            <td><tt>\x</tt></td></tr>
        <tr><th>2&nbsp;&nbsp;&nbsp;&nbsp;</th>
            <td>Range</td>
            <td><tt>a-z</tt></td></tr>
        <tr><th>3&nbsp;&nbsp;&nbsp;&nbsp;</th>
            <td>Grouping</td>
            <td><tt>[...]</tt></td></tr>
        <tr><th>4&nbsp;&nbsp;&nbsp;&nbsp;</th>
            <td>Intersection</td>
            <td><tt>[a-z&&[aeiou]]</tt></td></tr>
        <tr><th>5&nbsp;&nbsp;&nbsp;&nbsp;</th>
            <td>Union</td>
            <td><tt>[a-e][i-u]<tt></td></tr>
      </table></blockquote>

      <p> Note that a different set of metacharacters are in effect inside
      a character class than outside a character class. For instance, the
      regular expression <tt>.</tt> loses its special meaning inside a
      character class, while the expression <tt>-</tt> becomes a range
      forming metacharacter.

   <a name="lt">

   <a name="cg">
   <h4> Groups and capturing </h4>

   <p> Capturing groups are numbered by counting their opening parentheses from
   left to right.  In the expression <tt>((A)(B(C)))</tt>, for example, there
   are four such groups: </p>

   <blockquote><table cellpadding=1 cellspacing=0 summary="Capturing group numberings">

   <tr><th>1&nbsp;&nbsp;&nbsp;&nbsp;</th>
       <td><tt>((A)(B(C)))</tt></td></tr>
   <tr><th>2&nbsp;&nbsp;&nbsp;&nbsp;</th>
       <td><tt>(A)</tt></td></tr>
   <tr><th>3&nbsp;&nbsp;&nbsp;&nbsp;</th>
       <td><tt>(B(C))</tt></td></tr>

   <tr><th>4&nbsp;&nbsp;&nbsp;&nbsp;</th>
       <td><tt>(C)</tt></td></tr>
   </table></blockquote>

   <p> Group zero always stands for the entire expression.

   <p> Capturing groups are so named because, during a match, each subsequence
   of the input sequence that matches such a group is saved.  The captured
   subsequence may be used later in the expression, via a back reference, and
   may also be retrieved from the matcher once the match operation is complete.

   <p> The captured input associated with a group is always the subsequence
   that the group most recently matched.  If a group is evaluated a second time
   because of quantification then its previously-captured value, if any, will
   be retained if the second evaluation fails.  Matching the string
   <tt>"aba"</tt> against the expression <tt>(a(b)?)+</tt>, for example, leaves
   group two set to <tt>"b"</tt>.  All captured input is discarded at the
   beginning of each match.

   <p> Groups beginning with <tt>(?</tt> are pure, <i>non-capturing</i> groups
   that do not capture text and do not count towards the group total.


   <h4> Unicode support </h4>

   <p> Coming Soon.

   <h4> Comparison to Perl 5 </h4>

   <p>The <code>Pattern</code> engine performs traditional NFA-based matching
   with ordered alternation as occurs in Perl 5.

   <p> Perl constructs not supported by this class: </p>

   <ul>

      <li><p> The conditional constructs <tt>(?{</tt><i>X</i><tt>})</tt> and
      <tt>(?(</tt><i>condition</i><tt>)</tt><i>X</i><tt>|</tt><i>Y</i><tt>)</tt>,
      </p></li>

      <li><p> The embedded code constructs <tt>(?{</tt><i>code</i><tt>})</tt>
      and <tt>(??{</tt><i>code</i><tt>})</tt>,</p></li>

      <li><p> The embedded comment syntax <tt>(?#comment)</tt>, and </p></li>

      <li><p> The preprocessing operations <tt>\l</tt> <tt>&#92;u</tt>,
      <tt>\L</tt>, and <tt>\U</tt>.  </p></li>

      <li><p> Embedded flags</p></li>

   </ul>

   <p> Constructs supported by this class but not by Perl: </p>

   <ul>

      <li><p> Possessive quantifiers, which greedily match as much as they can
      and do not back off, even when doing so would allow the overall match to
      succeed.  </p></li>

      <li><p> Character-class union and intersection as described
      above.</p></li>

   </ul>

   <p> Notable differences from Perl: </p>

   <ul>

      <li><p> In Perl, <tt>\1</tt> through <tt>\9</tt> are always interpreted
      as back references; a backslash-escaped number greater than <tt>9</tt> is
      treated as a back reference if at least that many subexpressions exist,
      otherwise it is interpreted, if possible, as an octal escape.  In this
      class octal escapes must always begin with a zero. In this class,
      <tt>\1</tt> through <tt>\9</tt> are always interpreted as back
      references, and a larger number is accepted as a back reference if at
      least that many subexpressions exist at that point in the regular
      expression, otherwise the parser will drop digits until the number is
      smaller or equal to the existing number of groups or it is one digit.
      </p></li>

      <li><p> Perl uses the <tt>g</tt> flag to request a match that resumes
      where the last match left off.  This functionality is provided implicitly
      by the <CODE>Matcher</CODE> class: Repeated invocations of the
      <code>find</code> method will resume where the last match left off,
      unless the matcher is reset.  </p></li>

      <li><p> Perl is forgiving about malformed matching constructs, as in the
      expression <tt>*a</tt>, as well as dangling brackets, as in the
      expression <tt>abc]</tt>, and treats them as literals.  This
      class also strict and will not compile a pattern when dangling characters
      are encountered.</p></li>

   </ul>


   <p> For a more precise description of the behavior of regular expression
   constructs, please see <a href="http://www.oreilly.com/catalog/regex2/">
   <i>Mastering Regular Expressions, 2nd Edition</i>, Jeffrey E. F. Friedl,
   O'Reilly and Associates, 2002.</a>
   </p>
  <P>

  <i>End Text Extracted And Modified From java.util.regex.Pattern documentation</i>

  <hr>

  @author    Jeffery Stuart
  @since     March 2003, Stable Since November 2004
  @version   1.07.00
  @memo      A class used to represent "PERL 5"-ish regular expressions
 */
class Pattern
{
  friend class Matcher;
  friend class NFANode;
  friend class NFAQuantifierNode;
  private:
    /**
      This constructor should not be called directly. Those wishing to use the
      Pattern class should instead use the {@link compile compile} method.

      @param rhs The pattern to compile
      @memo Creates a new pattern from the regular expression in <code>rhs</code>.
     */
    Pattern(const bkstring & rhs);
  protected:
    /**
      This currently is not used, so don't try to do anything with it.
      @memo Holds all the compiled patterns for quick access.
     */
    static std::map<bkstring, Pattern *> compiledPatterns;
    /**
      Holds all of the registered patterns as strings. Due to certain problems
      with compilation of patterns, especially with capturing groups, this seemed
      to be the best way to do it.
     */
    static std::map<bkstring, std::pair<bkstring, unsigned long> > registeredPatterns;
  protected:
    /**
      Holds all the NFA nodes used. This makes deletion of a pattern, as well as
      clean-up from an unsuccessful compile much easier and faster.
     */
    std::map<NFANode*, bool> nodes;
    /**
      Used when methods like split are called. The matcher class uses a lot of
      dynamic memeory, so having an instance increases speedup of certain
      operations.
     */
    Matcher * matcher;
    /**
      The front node of the NFA.
     */
    NFANode * head;
    /**
      The actual regular expression we rerpesent
      */
    bkstring pattern;
    /**
      Flag used during compilation. Once the pattern is successfully compiled,
      <code>error</code> is no longer used.
     */
    bool error;
    /**
      Used during compilation to keep track of the current index into
      <code>{@link pattern pattern}<code>.  Once the pattern is successfully
      compiled, <code>error</code> is no longer used.
     */
    int curInd;
    /**
      The number of capture groups this contains.
     */
    int groupCount;
    /**
      The number of non-capture groups this contains.
     */
    int nonCapGroupCount;
    /**
      The flags specified when this was compiled.
     */
    unsigned long flags;
  protected:
    /**
      Raises an error during compilation. Compilation will cease at that point
      and compile will return <code>NULL</code>.
     */
    void raiseError();
    /**
      Convenience function for registering a node in <code>nodes</code>.
      @param node The node to register
      @return The registered node
     */
    NFANode * registerNode(NFANode * node);

    /**
      Calculates the union of two strings. This function will first sort the
      strings and then use a simple selection algorithm to find the union.
      @param s1 The first "class" to union
      @param s2 The second "class" to union
      @return A new string containing all unique characters. Each character
              must have appeared in one or both of <code>s1</code> and
              <code>s2</code>.
     */
    bkstring classUnion      (bkstring s1, bkstring s2)  const;
    /**
      Calculates the intersection of two strings. This function will first sort
      the strings and then use a simple selection algorithm to find the
      intersection.
      @param s1 The first "class" to intersect
      @param s2 The second "class" to intersect
      @return A new string containing all unique characters. Each character
              must have appeared both <code>s1</code> and <code>s2</code>.
     */
    bkstring classIntersect  (bkstring s1, bkstring s2)  const;
    /**
      Calculates the negation of a string. The negation is the set of all
      characters between <code>\x00</code> and <code>\xFF</code> not
      contained in <code>s1</code>.
      @param s1 The "class" to be negated.
      @param s2 The second "class" to intersect
      @return A new string containing all unique characters. Each character
              must have appeared both <code>s1</code> and <code>s2</code>.
     */
    bkstring classNegate     (bkstring s1)                  const;
    /**
      Creates a new "class" representing the range from <code>low</code> thru
      <code>hi</code>. This function will wrap if <code>low</code> &gt;
      <code>hi</code>. This is a feature, not a buf. Sometimes it is useful
      to be able to say [\x70-\x10] instead of [\x70-\x7F\x00-\x10].
      @param low The beginning character
      @param hi  The ending character
      @return A new string containing all the characters from low thru hi.
     */
    bkstring classCreateRange(char low,       char hi)         const;

    /**
      Extracts a decimal number from the substring of member-variable
      <code>{@link pattern pattern}<code> starting at <code>start</code> and
      ending at <code>end</code>.
      @param start The starting index in <code>{@link pattern pattern}<code>
      @param end The last index in <code>{@link pattern pattern}<code>
      @return The decimal number in <code>{@link pattern pattern}<code>
     */
    int getInt(int start, int end);
    /**
      Parses a <code>{n,m}</code> string out of the member-variable
      <code>{@link pattern pattern}<code> stores the result in <code>sNum</code>
      and <code>eNum</code>.
      @param sNum Output parameter. The minimum number of matches required
                  by the curly quantifier are stored here.
      @param eNum Output parameter. The maximum number of matches allowed
                  by the curly quantifier are stored here.
      @return Success/Failure. Fails when the curly does not have the proper
              syntax
     */
    bool quantifyCurly(int & sNum, int & eNum);
    /**
      Tries to quantify the currently parsed group. If the group being parsed
      is indeed quantified in the member-variable
      <code>{@link pattern pattern}<code>, then the NFA is modified accordingly.
      @param start  The starting node of the current group being parsed
      @param stop   The ending node of the current group being parsed
      @param gn     The group number of the current group being parsed
      @return       The node representing the starting node of the group. If the
                    group becomes quantified, then this node is not necessarily
                    a GroupHead node.
     */
    NFANode * quantifyGroup(NFANode * start, NFANode * stop, const int gn);

    /**
      Tries to quantify the last parsed expression. If the character was indeed
      quantified, then the NFA is modified accordingly.
      @param newNode The recently created expression node
      @return The node representing the last parsed expression. If the
              expression was quantified, <code>return value != newNode</code>
     */
    NFANode * quantify(NFANode * newNode);
    /**
      Parses the current class being examined in
      <code>{@link pattern pattern}</code>.
      @return A string of unique characters contained in the current class being
              parsed
     */
    bkstring parseClass();
    /**
      Parses the current POSIX class being examined in
      <code>{@link pattern pattern}</code>.
      @return A string of unique characters representing the POSIX class being
              parsed
     */
    bkstring parsePosix();
    /**
      Returns a string containing the octal character being parsed
      @return The string contained the octal value being parsed
     */
    bkstring parseOctal();
    /**
      Returns a string containing the hex character being parsed
      @return The string contained the hex value being parsed
     */
    bkstring parseHex();
    /**
      Returns a new node representing the back reference being parsed
      @return The new node representing the back reference being parsed
     */
    NFANode *   parseBackref();
    /**
      Parses the escape sequence currently being examined. Determines if the
      escape sequence is a class, a single character, or the beginning of a
      quotation sequence.
      @param inv Output parameter. Whether or not to invert the returned class
      @param quo Output parameter. Whether or not this sequence starts a
                 quotation.
      @return The characters represented by the class
     */
    bkstring parseEscape(bool & inv, bool & quo);
    /**
      Parses a supposed registered pattern currently under compilation. If the
      sequence of characters does point to a registered pattern, then the
      registered pattern is appended to <code>*end<code>. The registered pattern
      is parsed with the current compilation flags.
      @param end The ending node of the thus-far compiled pattern
      @return The new end node of the current pattern
     */
    NFANode * parseRegisteredPattern(NFANode ** end);
    /**
      Parses a lookbehind expression. Appends the necessary nodes
      <code>*end</code>.
      @param pos Positive or negative look behind
      @param end The ending node of the current pattern
      @return The new end node of the current pattern
     */
    NFANode * parseBehind(const bool pos, NFANode ** end);
    /**
      Parses the current expression and tacks on nodes until a \E is found.
      @return The end of the current pattern
     */
    NFANode * parseQuote();
    /**
      Parses <code>{@link pattern pattern}</code>. This function is called
      recursively when an or (<code>|</code>) or a group is encountered.
      @param inParen Are we currently parsing inside a group
      @param inOr Are we currently parsing one side of an or (<code>|</code>)
      @param end The end of the current expression
      @return The starting node of the NFA constructed from this parse
     */
    NFANode * parse(const bool inParen = 0, const bool inOr = 0, NFANode ** end = NULL);
  public:
    /// We should match regardless of case
    const static unsigned long CASE_INSENSITIVE;
    /// We are implicitly quoted
    const static unsigned long LITERAL;
    /// @memo We should treat a <code><b>.</b></code> as [\x00-\x7F]
    const static unsigned long DOT_MATCHES_ALL;
    /** <code>^</code> and <code>$</code> should anchor to the beginning and
        ending of lines, not all input
     */
    const static unsigned long MULTILINE_MATCHING;
    /** When enabled, only instances of <code>\n</codes> are recognized as
        line terminators
     */
    const static unsigned long UNIX_LINE_MODE;
    /// The absolute minimum number of matches a quantifier can match (0)
    const static int MIN_QMATCH;
    /// The absolute maximum number of matches a quantifier can match (0x7FFFFFFF)
    const static int MAX_QMATCH;
  public:
    /**
      Call this function to compile a regular expression into a
      <code>Pattern</code> object. Special values can be assigned to
      <code>mode</code> when certain non-standard behaviors are expected from
      the <code>Pattern</code> object.
      @param pattern The regular expression to compile
      @param mode    A bitwise or of flags signalling what special behaviors are
                     wanted from this <code>Pattern</code> object
      @return If successful, <code>compile</code> returns a <code>Pattern</code>
              pointer. Upon failure, <code>compile</code> returns
              <code>NULL</code>
     */
    static Pattern                    * compile        (const bkstring & pattern,
                                                        const unsigned long mode = 0);
    /**
      Dont use this function. This function will compile a pattern, and cache
      the result. This will eventually be used as an optimization when people
      just want to call static methods using the same pattern over and over
      instead of first compiling the pattern and then using the compiled
      instance for matching.
      @param pattern The regular expression to compile
      @param mode    A bitwise or of flags signalling what special behaviors are
                     wanted from this <code>Pattern</code> object
      @return If successful, <code>compileAndKeep</code> returns a
              <code>Pattern</code> pointer. Upon failure, <code>compile</code>
              returns <code>NULL</code>.
     */
    static Pattern                    * compileAndKeep (const bkstring & pattern,
                                                        const unsigned long mode = 0);

    /**
      Searches through <code>replace</code> and replaces all substrings matched
      by <code>pattern</code> with <code>str</code>. <code>str</code> may
      contain backreferences (e.g. <code>\1</code>) to capture groups. A typical
      invocation looks like:
      <p>
      <code>
      Pattern::replace("(a+)b(c+)", "abcccbbabcbabc", "\\2b\\1");
      </code>
      <p>
      which would replace <code>abcccbbabcbabc</code> with
      <code>cccbabbcbabcba</code>.
      @param pattern          The regular expression
      @param str              The replacement text
      @param replacementText  The string in which to perform replacements
      @param mode             The special mode requested of the <code>Pattern</code>
                              during the replacement process
      @return The text with the replacement string substituted where necessary
     */
    static bkstring                  replace        (const bkstring & pattern,
                                                        const bkstring & str,
                                                        const bkstring & replacementText,
                                                        const unsigned long mode = 0);

    /**
      Splits the specified string over occurrences of the specified pattern.
      Empty strings can be optionally ignored. The number of strings returned is
      configurable. A typical invocation looks like:
      <p>
      <code>
      bkstring str(strSize, '\0');<br>
      FILE * fp = fopen(fileName, "r");<br>
      fread((char*)str.data(), strSize, 1, fp);<br>
      fclose(fp);<br>
      <br>
      std::vector&lt;bkstring&gt; lines = Pattern::split("[\r\n]+", str, true);<br>
      <br>
      </code>

      @param pattern    The regular expression
      @param replace    The string to split
      @param keepEmptys Whether or not to keep empty strings
      @param limit      The maximum number of splits to make
      @param mode       The special mode requested of the <code>Pattern</code>
                        during the split process
      @return All substrings of <code>str</code> split across <code>pattern</code>.
     */
    static std::vector<bkstring>     split          (const bkstring & pattern,
                                                        const bkstring & str,
                                                        const bool keepEmptys = 0,
                                                        const unsigned long limit = 0,
                                                        const unsigned long mode = 0);

    /**
      Finds all the instances of the specified pattern within the string. You
      should be careful to only pass patterns with a minimum length of one. For
      example, the pattern <code>a*</code> can be matched by an empty string, so
      instead you should pass <code>a+</code> since at least one character must
      be matched. A typical invocation of <code>findAll</code> looks like:
      <p>
      <code>
      std::vector&lt;td::string&gt; numbers = Pattern::findAll("\\d+", string);
      </code>
      <p>

      @param pattern  The pattern for which to search
      @param str      The string to search
      @param mode     The special mode requested of the <code>Pattern</code>
                      during the find process
      @return All instances of <code>pattern</code> in <code>str</code>
     */
    static std::vector<bkstring>     findAll        (const bkstring & pattern,
                                                        const bkstring & str,
                                                        const unsigned long mode = 0);

    /**
      Determines if an entire string matches the specified pattern

      @param pattern  The pattern for to match
      @param str      The string to match
      @param mode     The special mode requested of the <code>Pattern</code>
                      during the replacement process
      @return True if <code>str</code> is recognized by <code>pattern</code>
     */
    static bool                         matches        (const bkstring & pattern,
                                                        const bkstring & str,
                                                        const unsigned long mode = 0);

    /**
      Registers a pattern under a specific name for use in later compilations.
      A typical invocation and later use looks like:
      <p>
      <code>
      Pattern::registerPattern("ip", "(?:\\d{1,3}\\.){3}\\d{1,3}");<br>
      Pattern * p1 = Pattern::compile("{ip}:\\d+");<br>
      Pattern * p2 = Pattern::compile("Connection from ({ip}) on port \\d+");<br>
      </code>
      <p>
      Multiple calls to <code>registerPattern</code> with the same
      <code>name</code> will result in the pattern getting overwritten.

      @param name     The name to give to the pattern
      @param pattern  The pattern to register
      @param mode     Any special flags to use when compiling pattern
      @return Success/Failure. Fails only if <code>pattern</code> has invalid
              syntax
     */
    static bool                         registerPattern(const bkstring & name,
                                                        const bkstring & pattern,
                                                        const unsigned long mode = 0);

    /**
      Clears the pattern registry
      */
    static void                         unregisterPatterns();
    /**
      Don't use
     */
    static void                         clearPatternCache();

    /**
      Searches through a string for the <code>n<sup>th</sup></code> match of the
      given pattern in the string. Match indeces start at zero, not one.
      A typical invocation looks like this:
      <p>
      <code>
      std::pair&lt;bkstring, int&gt; match = Pattern::findNthMatch("\\d{1,3}", "192.168.1.101:22", 1);<br>
      printf("%s %i\n", match.first.c_str(), match.second);<br>
      <br>
      Output: 168 4<br>
      <br>

      @param pattern  The pattern for which to search
      @param str      The string to search
      @param matchNum Which match to find
      @param mode     Any special flags to use during the matching process
      @return A string and an integer. The string is the string matched. The
              integer is the starting location of the matched string in
              <code>str</code>. You can check for success/failure by making sure
              that the integer returned is greater than or equal to zero.
     */
    static std::pair<bkstring, int>  findNthMatch   (const bkstring & pattern,
                                                        const bkstring & str,
                                                        const int matchNum,
                                                        const unsigned long mode = 0);
  public:
    /**
      Deletes all NFA nodes allocated during compilation
     */
    ~Pattern();

    bkstring               replace        (const bkstring & str,
                                              const bkstring & replacementText);
    std::vector<bkstring>  split          (const bkstring & str, const bool keepEmptys = 0,
                                              const unsigned long limit = 0);
    std::vector<bkstring>  findAll        (const bkstring & str);
    bool                      matches        (const bkstring & str);
    /**
      Returns the flags used during compilation of this pattern
      @return The flags used during compilation of this pattern
     */
    unsigned long             getFlags       () const;
    /**
      Returns the regular expression this pattern represents
      @return The regular expression this pattern represents
     */
    bkstring               getPattern     () const;
    /**
      Creates a matcher object using the specified string and this pattern.
      @param str The string to match against
      @return A new matcher using object using this pattern and the specified
              string
     */
    Matcher                 * createMatcher  (const bkstring & str);
};

class NFANode
{
  friend class Matcher;
  public:
    NFANode * next;
    NFANode();
    virtual ~NFANode();
    virtual void findAllNodes(std::map<NFANode*, bool> & soFar);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const = 0;
    inline virtual bool isGroupHeadNode()     const { return false; }
    inline virtual bool isStartOfInputNode()  const { return false; }
};
class NFACharNode : public NFANode
{
  protected:
    char ch;
  public:
    NFACharNode(const char c);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFACICharNode : public NFANode
{
  protected:
    char ch;
  public:
    NFACICharNode(const char c);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAStartNode : public NFANode
{
  public:
    NFAStartNode();
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAEndNode : public NFANode
{
  public:
    NFAEndNode();
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAQuantifierNode : public NFANode
{
  public:
    int min, max;
    NFANode * inner;
    virtual void findAllNodes(std::map<NFANode*, bool> & soFar);
    NFAQuantifierNode(Pattern * pat, NFANode * internal,
                      const int minMatch = Pattern::MIN_QMATCH,
                      const int maxMatch = Pattern::MAX_QMATCH);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAGreedyQuantifierNode : public NFAQuantifierNode
{
  public:
    NFAGreedyQuantifierNode(Pattern * pat, NFANode * internal,
                            const int minMatch = Pattern::MIN_QMATCH,
                            const int maxMatch = Pattern::MAX_QMATCH);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
    virtual int matchInternal(const bkstring & str, Matcher * matcher, const int curInd, const int soFar) const;
};
class NFALazyQuantifierNode : public NFAQuantifierNode
{
  public:
    NFALazyQuantifierNode(Pattern * pat, NFANode * internal,
                          const int minMatch = Pattern::MIN_QMATCH,
                          const int maxMatch = Pattern::MAX_QMATCH);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAPossessiveQuantifierNode : public NFAQuantifierNode
{
  public:
    NFAPossessiveQuantifierNode(Pattern * pat, NFANode * internal,
                                const int minMatch = Pattern::MIN_QMATCH,
                                const int maxMatch = Pattern::MAX_QMATCH);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAAcceptNode : public NFANode
{
  public:
    NFAAcceptNode();
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAClassNode : public NFANode
{
  public:
    bool inv;
    std::map<char, bool> vals;
    NFAClassNode(const bool invert = 0);
    NFAClassNode(const bkstring & clazz, const bool invert);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFACIClassNode : public NFANode
{
  public:
    bool inv;
    std::map<char, bool> vals;
    NFACIClassNode(const bool invert = 0);
    NFACIClassNode(const bkstring & clazz, const bool invert);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFASubStartNode : public NFANode
{
  public:
    NFASubStartNode();
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAOrNode : public NFANode
{
  public:
    NFANode * one;
    NFANode * two;
    NFAOrNode(NFANode * first, NFANode * second);
    virtual void findAllNodes(std::map<NFANode*, bool> & soFar);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAQuoteNode : public NFANode
{
  public:
    bkstring qStr;
    NFAQuoteNode(const bkstring & quoted);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFACIQuoteNode : public NFANode
{
  public:
    bkstring qStr;
    NFACIQuoteNode(const bkstring & quoted);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFALookAheadNode : public NFANode
{
  public:
    bool pos;
    NFANode * inner;
    NFALookAheadNode(NFANode * internal, const bool positive);
    virtual void findAllNodes(std::map<NFANode*, bool> & soFar);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFALookBehindNode : public NFANode
{
  public:
    bool pos;
    bkstring mStr;
    NFALookBehindNode(const bkstring & str, const bool positive);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAStartOfLineNode : public NFANode
{
  public:
    NFAStartOfLineNode();
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAEndOfLineNode : public NFANode
{
  public:
    NFAEndOfLineNode();
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAReferenceNode : public NFANode
{
  public:
    int gi;
    NFAReferenceNode(const int groupIndex);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAStartOfInputNode : public NFANode
{
  public:
    NFAStartOfInputNode();
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
    inline virtual bool isStartOfInputNode()  const { return true; }
};
class NFAEndOfInputNode : public NFANode
{
  public:
    bool term;
    NFAEndOfInputNode(const bool lookForTerm);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAWordBoundaryNode : public NFANode
{
  public:
    bool pos;
    NFAWordBoundaryNode(const bool positive);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAEndOfMatchNode : public NFANode
{
  public:
    NFAEndOfMatchNode();
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAGroupHeadNode : public NFANode
{
  public:
    int gi;
    NFAGroupHeadNode(const int groupIndex);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
    inline virtual bool isGroupHeadNode()     const { return true; }
};
class NFAGroupTailNode : public NFANode
{
  public:
    int gi;
    NFAGroupTailNode(const int groupIndex);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAGroupLoopPrologueNode : public NFANode
{
  public:
    int gi;
    NFAGroupLoopPrologueNode(const int groupIndex);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};
class NFAGroupLoopNode : public NFANode
{
  public:
    int gi, min, max, type;
    NFANode * inner;
    NFAGroupLoopNode(NFANode * internal, const int minMatch,
                     const int maxMatch, const int groupIndex, const int matchType);
    virtual void findAllNodes(std::map<NFANode*, bool> & soFar);
    virtual int match(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
    int matchGreedy(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
    int matchLazy(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
    int matchPossessive(const bkstring & str, Matcher * matcher, const int curInd = 0) const;
};

#endif

