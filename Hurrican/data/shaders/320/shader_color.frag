/* Input */
in vec4 v_Color;
/* Output */
varying out vec4 v_FragColor;

void main()
{
    /* Use the current color */
    v_FragColor = v_Color;
}
