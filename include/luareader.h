
#ifndef __LUAREADER_H_INCLUDED__
#define __LUAREADER_H_INCLUDED__


#ifdef __cplusplus
extern "C" {
#endif

#ifndef LUAREADER_API
#ifdef WIN32
#ifdef LUAREADER_EXPORTS
#define LUAREADER_API __declspec(dllexport)
#else
#define LUAREADER_API __declspec(dllimport)
#endif
#else
#define LUAREADER_API
#endif
#endif

/* �ű��ص�Ӧ�ó���
 context�� ͬluareader_new����ֵ
 command������
 input(input_len)����������
 output(max_output_size,result)���������
 result��������ݳ���
 example���ڽű���ִ��	luareader_callback('log', 'test')
*/
typedef int (*luareader_callback_function)(void *context, const char *command, const unsigned char * input, int input_len, unsigned char * output, int max_output_size);

/* �������󣨼��ء�ִ�нű������ûص���
 flags��1-scriptΪ�ű��ļ�
 script��lua�ű�/�ļ���windows��Ƕsysapi����linux��Ƕusb����
 callback��C�ص�����
 result��lua������
*/
LUAREADER_API void * luareader_new(int flags, const char *script, luareader_callback_function callback);
/* ɾ������
 context��������
 result��[0]�ɹ���[-1001]��������Ч
*/
LUAREADER_API int luareader_term(void *context);

/* ��ȡ�������б��ڲ�����luareader_do_task��
 context��������
 reader_names(max_reader_names_size,result)������������б����ַ�����ʽ��
 result��[>0]������ȣ�[-1001]��������Ч��[-1,-6]ִ�нű����󣨱������һ��luareader_pop_value��ȡ������Ϣ��
*/
LUAREADER_API int luareader_get_list(void *context, char * reader_names, int max_reader_names_size);

/* ���Ӷ��������ڲ�����luareader_do_task��
 context��������
 reader_name������������
 result��[0]�ɹ���[-1001]��������Ч��[-1,-6]ִ�нű����󣨱������һ��luareader_pop_value��ȡ������Ϣ��
*/
LUAREADER_API int luareader_connect(void *context, const char * reader_name);

/* ��������ִ��ָ��ڲ�����luareader_do_task��
 context��������
 apdu(apdu_len)��APDU����
 resp(max_resp_size,result)��ָ��ִ�н��
 result��[>0]������ȣ�[-1001]��������Ч��[-1,-6]ִ�нű����󣨱������һ��luareader_pop_value��ȡ������Ϣ��
*/
LUAREADER_API int luareader_transmit(void *context, const unsigned char * apdu, int apdu_len, unsigned char * resp, int max_resp_size);

/* �Ͽ������������ӣ��ڲ�����luareader_do_task��
 context��������
 result��[0]�ɹ���[-1001]��������Ч��[-1,-6]ִ�нű����󣨱������һ��luareader_pop_value��ȡ������Ϣ��
*/
LUAREADER_API int luareader_disconnect(void *context);

/* ִ��ָ������
 context��������
 tast_name���������ƣ��ű���������
 input(input_len)����������
 output(max_output_size,result)��������
 result��[>=0]������ȣ�[-1001]��������Ч��[-1,-6]ִ�нű����󣨱������һ��luareader_pop_value��ȡ������Ϣ��
*/
LUAREADER_API int luareader_do_task(void *context, const char * tast_name, const unsigned char * input, int input_len, unsigned char * output, int max_output_size);

/* ִ�нű�
 context��������
 str���ű�
 output(max_output_size,result)��������
 result��[>=0]������ȣ�[-1001]��������Ч��[-1,-6]ִ�нű����󣨱������һ��luareader_pop_value��ȡ������Ϣ��
*/
LUAREADER_API int luareader_do_string(void *context, const char * str, unsigned char * output, int max_output_size);

/* ����ջ����ֵ
 context��������
 value(max_value_size,result)��ֵ
 result��[0]������ȣ�[-1001]��������Ч��[-1003]��ջ
*/
LUAREADER_API int luareader_pop_value(void *context, char * value, int max_value_size);


#ifdef __cplusplus
}
#endif

#endif
