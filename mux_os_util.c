
#include "mux_os_type.h"
#include "mux_os_api.h"
#include <stdarg.h>
#ifdef OS_SELF_PRINTF

//缓冲区大小
#define OS_UTIL_PRINTF_BUFF_LEN 4096
char os_util_debug_buff[OS_UTIL_PRINTF_BUFF_LEN];//缓冲区

#define SIGN 16

static const char   DIGIT[17]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','-'};
static const char DIGIT_X[17]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','-'};

int os_vsnprintf (char *buffer, int n, const char *fmt, va_list ap );

int os_vsprintf (char *buffer, const char *fmt, va_list ap )
{
    return os_vsnprintf(buffer, OS_UTIL_PRINTF_BUFF_LEN, fmt, ap);
}
//格式化数据并发送出去
int os_util_printf(const char *fmt,...)
{
    va_list a;
    int length;

	os_mem_set(os_util_debug_buff,0,OS_UTIL_PRINTF_BUFF_LEN);
    va_start (a, fmt);
    length = os_vsprintf (os_util_debug_buff, fmt, a);
    va_end (a);

#ifdef OS_ENVIRONMENT_WIN32
	os_util_debug_buff[length] = '\n';
	printf(os_util_debug_buff);
#endif

#ifdef OS_ENVIRONMENT_LINUX
	os_util_debug_buff[length] = '\n';
	printf(os_util_debug_buff);
#endif
		
    return length;
}
//格式化数据并存储到给定的存储区
int os_sprintf(char *buff, const char *fmt, ...)
{
	va_list a;
    int length;

    va_start (a, fmt);
    length = os_vsprintf (buff, fmt, a);
    va_end (a);

    return length;
}

/*
c	: 
u	: 
d,i	: 
X	: 
p	:
x	:
s	:
m,M : char * , int 
char * 	: 所要打印的缓冲区指针
int 	: 所要打印的长度
*/
int os_vsnprintf (char *buffer, int n, const char *fmt, va_list ap )
{
    int  	 n_dest = n;
    char     *p, *sval;
    int      ival,digit;
	unsigned int index;
    char     char_val;
    char     *p_dest=buffer;
    unsigned int     uint_val;
    char     digit_buf[40];
    unsigned int     length;
    char     font_char;
    int      sign;
    unsigned long      ulong_val;
    long      long_val;
    unsigned char l = 0;

#define deststore(c) { if(n_dest > 1) { n_dest--; *p_dest = c; } p_dest++; }
    
    for (p = (char *) fmt; *p; p++)
    {
        if (*p != '%')
        {
            deststore(*p);
            continue;
        }
        p++;
        while ((*p=='-') || (*p=='+')) p++;
        length=0;
        if (*p == '0') 
        {
            font_char = '0'; p++;
        }
        else
        {
            font_char = ' ';
        }
        
        while (isdigit(*p))
        {
            length = length * 10 + (*p++ - '0');
        }

        if (*p == 'l' && *(p + 1) != '\0') 
        {
            l = 1;
            p++;
        }

        switch (*p)
        {
            case 'c':
            {
                l = 0;
                char_val = va_arg(ap, int);
                deststore(char_val);
                break;
            }
            
            case 'u':
            {
                if ( l )
                {
                    l=0;
                    ulong_val = va_arg(ap, unsigned long);
                    index=0;
                    if (ulong_val == 0) 
                    {
                        digit_buf[index++]=0;
                    }
                    else
                    {
                        while (ulong_val != 0)
                        {
                            digit=ulong_val % 10;
                            digit_buf[index++]=digit;
                            ulong_val-=digit;
                            ulong_val/=10;
                        };
                    };
                    if (index < length) 
                    {
                        length-=index; 
                        while (length-- != 0) 
                        {
                            deststore(font_char);
                        };
                    };
                    while (index--!=0) 
                    {
                        deststore(DIGIT[digit_buf[index]]);
                    };
                }
                else
                {
                    uint_val = va_arg(ap, unsigned int);
                    index=0;
                    if (uint_val == 0) 
                    {
                        digit_buf[index++]=0;
                    }
                    else
                    {
                        while (uint_val != 0)
                        {
                            digit=uint_val % 10;
                            digit_buf[index++]=digit;
                            uint_val-=digit;
                            uint_val/=10;
                        };
                    };
                    if (index < length) 
                    {
                        length-=index; 
                        while (length-- != 0) 
                        {
                            deststore(font_char);
                        };
                    };
                    while (index--!=0) 
                    {
                        deststore(DIGIT[digit_buf[index]]);
                    };
                }
                break;
            }
            
            case 'd':
            case 'i':
            {
                if ( l )
                {
                    l=0;
                    long_val = va_arg(ap, long);
                    index=0;
                    if (long_val == 0) 
                    {
                        digit_buf[index++]=0;
                    }
                    else
                    {
                        if (long_val<0) {long_val=-long_val; sign=-1;}
                        else 
                        {
                            sign = 0;
                        };
                        while (long_val != 0)
                        {
                            digit=long_val % 10;
                            digit_buf[index++]=digit;
                            long_val-=digit;
                            long_val/=10;
                        };
                        if (sign == -1) {digit_buf[index++] = SIGN;};
                    }
                    
                    if (index < length) 
                    {
                        length-=index; 
                        while (length-- != 0) 
                        {
                            deststore(font_char);
                        };
                    };
                    while (index--!=0) 
                    {
                        deststore(DIGIT[digit_buf[index]]);
                    };
                }
                else
                {
                    ival = va_arg(ap, int);
                    index=0;
                    if (ival == 0) 
                    {
                        digit_buf[index++]=0;
                    }
                    else
                    {
                        if (ival<0) {ival=-ival; sign=-1;}
                        else 
                        {
                            sign = 0;
                        };
                        while (ival != 0)
                        {
                            digit=ival % 10;
                            digit_buf[index++]=digit;
                            ival-=digit;
                            ival/=10;
                        };
                        if (sign == -1) {digit_buf[index++] = SIGN;};
                    };
                    if (index < length) 
                    {
                        length-=index; 
                        while (length-- != 0) 
                        {
                            deststore(font_char);
                        };
                    };
                    while (index--!=0) 
                    {
                        deststore(DIGIT[digit_buf[index]]);
                    };
                }
                break;

            }

            case 'X':
            {
                if (l)
                {
                    l = 0;
                    ulong_val = va_arg(ap, unsigned long);
                    index = 0;
                    if (ulong_val == 0) 
                    {
                        digit_buf[index++]=0;
                    }
                    else
                    {
                        while(ulong_val != 0)
                        {   
                            digit_buf[index++]=ulong_val & 0x0f;
                            ulong_val = ulong_val >> 4;
                        }
                    };
                    if (index < length) 
                    {
                        length-=index; 
                        while (length-- != 0) 
                        {
                            deststore(font_char);
                        };
                    };
                    while (index--!=0) 
                    {
                        deststore(DIGIT_X[digit_buf[index]]);
                    }
                }
                else
                {
                    uint_val = va_arg(ap, unsigned int);
                    index = 0;
                    if (uint_val == 0) 
                    {
                        digit_buf[index++]=0;
                    }
                    else
                    {
                        while(uint_val != 0)
                        {   
                            digit_buf[index++]=uint_val & 0x0f;
                            uint_val = uint_val >> 4;
                        }
                    };
                    if (index < length) 
                    {
                        length-=index; 
                        while (length-- != 0) 
                        {
                            deststore(font_char);
                        };
                    };
                    while (index--!=0) 
                    {
                        deststore(DIGIT_X[digit_buf[index]]);
                    }
                }

                break;

            }
            
            case 'p':
                deststore('0');
                deststore('x');
                l = 1;
            case 'x':
            {
                if (l)
                {
                    l = 0;
                    ulong_val = va_arg(ap, unsigned long);
                    index=0;
                    if (ulong_val==0) 
                    {
                        digit_buf[index++]=0;
                    }
                    else
                    {
                        while(ulong_val != 0)
                        {   
                            digit_buf[index++]=((char)ulong_val) & 0x0f;
                            ulong_val = ulong_val >> 4;
                        }
                    };
                    if (index < length) 
                    {
                        length-=index; 
                        while (length-- != 0) 
                        {
                            deststore(font_char);
                        };
                    };
                    while (index--!=0) 
                    {
                        deststore(DIGIT[digit_buf[index]]);
                    };
                }
                else
                {
                    uint_val = va_arg(ap, unsigned int);
                    index = 0;
                    if (uint_val == 0) 
                    {
                        digit_buf[index++]=0;
                    }
                    else
                    {
                        while(uint_val != 0)
                        {   
                            digit_buf[index++]=uint_val & 0x0f;
                            uint_val = uint_val >> 4;
                        }
                    };
                    if (index < length) 
                    {
                        length-=index; 
                        while (length-- != 0) 
                        {
                            deststore(font_char);
                        };
                    };
                    while (index--!=0) 
                    {
                        deststore(DIGIT[digit_buf[index]]);
                    }
                }
                break;
            }
            
            case 's':
            {
                l = 0;
                sval = va_arg(ap, char *);
                if (length != 0)
                {
                    index = strlen( sval);
                    if (index < length) 
                    {
                        length-=index; 
                        while (length-- != 0) 
                        {
                            deststore(font_char);
                        };
                    };
                };
                for(;*sval; sval++) 
                    deststore(*sval);
                break;
            }
            case 'm':
            case 'M':
            	sval=va_arg(ap, char *);
            	ival=va_arg(ap, int);
            	
            	deststore('0');
            	deststore('x');
            	
            	for(index=0; index<ival; index++)
            	{
            		deststore((char)DIGIT_X[(sval[index]>>4)&0x0F]);
            		deststore((char)DIGIT_X[sval[index]&0x0F]);
            		
            		deststore(' ');
            	}
            
            	break;
            default:
            {
                deststore(*p);
                break;
            }
        }
        if (*p == '\0')
            break;
    }

    n_dest = p_dest - buffer;

    if(n_dest >= n) 
    {
        if(n > 0)
            buffer[n - 1] = '\0';
    }
    else
        *p_dest = '\0';

    return n_dest;
}

#endif

