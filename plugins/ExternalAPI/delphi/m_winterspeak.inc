{
}
const
{
  wParam - contact Handle
  lParam - Unicode text
  Notes: difference of services is for "ignore" checks, status or messages
  offline contacts don't processes (use hCantact = 0 instead)
  if English voice choosen, it can't process non-English texts (silence on Russian, for example)
}
  MS_SPEAK_STATUS :PAnsiChar = 'Speak/Status';
  MS_SPEAK_MESSAGE:PAnsiChar = 'Speak/Message';
