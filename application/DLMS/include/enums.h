//
// --------------------------------------------------------------------------
//  Gurux Ltd
//
//
//
// Filename:        $HeadURL:  $
//
// Version:         $Revision:  $,
//                  $Date:  $
//                  $Author: $
//
// Copyright (c) Gurux Ltd
//
//---------------------------------------------------------------------------

#ifndef ENUMS_H
#define ENUMS_H

#define DLMS_PROTOCOL_VERSION       6

  // Server sender frame sequence starting number.
  static const unsigned char SERVER_START_SENDER_FRAME_SEQUENCE = 0x1E;

  // Server receiver frame sequence starting number.
  static const unsigned char SERVER_START_RECEIVER_FRAME_SEQUENCE = 0x0E;

  // Client sender frame sequence starting number.
  static const unsigned char CLIENT_START_SENDER_FRAME_SEQUENCE = 0x10;

  // Client receiver frame sequence starting number.
  static const unsigned char CLIENT_START_RCEIVER_FRAME_SEQUENCE = 0xE;

  static const unsigned char HDLCFrameType = 0xA0;
  static const unsigned char HDLCFrameTypeMoreData = 0xA8;
  static const unsigned char HDLC_FRAME_START_END = 0x7E;
  static const char AARQTag = 0x60;
  static const char AARETag = 0x61;
  static const char DLMSVersioNumber = 6;

  typedef enum
  {
    TAG_NULLDATA = 0,
    TAG_ARRAY = 1,
    TAG_STRUCTURE = 2,
    TAG_BOOLEAN = 3,
    TAG_BITSTRING = 4,
    TAG_INT32 = 5,
    TAG_UINT32 = 6,
    TAG_FLOATING_POINT = 7,
    TAG_OCTET_STRING = 9,
    TAG_VISIBLE_STRING = 10,
    TAG_GENERALIZED_TIME = 11,
    TAG_BCD = 13,
    TAG_INT8 = 15,
    TAG_INT16 = 16,
    TAG_UINT8 = 17,
    TAG_UINT16 = 18,
    TAG_COMPACT_ARRAY = 19,
    TAG_INT64 = 20,
    TAG_UINT64 = 21,
    TAG_ENUM = 22,
    TAG_FLOAT32 = 23,
    TAG_FLOAT64 = 24,
    TAG_DATETIME = 25,
    TAG_DATE = 26,
    TAG_TIME = 27,
    TAG_DONTCARE = 255
  }DLMS_datatpyes;
  
  typedef enum
  {
    DLMS_ASSOCIATION_RESULT_ACCEPTED = 0,
    DLMS_ASSOCIATION_RESULT_PERMANENT_REJECTED = 1,
    DLMS_ASSOCIATION_RESULT_TRANSIENT_REJECTED = 2
  } DLMS_ASSOCIATION_RESULT;

  typedef enum
  {
    DLMS_SOURCE_DIAGNOSTIC_NONE = 0,
    DLMS_SOURCE_DIAGNOSTIC_NO_REASON_GIVEN = 1,
    DLMS_SOURCE_DIAGNOSTIC_APPLICATION_CONTEXT_NAME_NOT_SUPPORTED = 2,
    DLMS_SOURCE_DIAGNOSTIC_AUTHENTICATION_MECHANISM_NAME_NOT_RECOGNISED = 11,
    DLMS_SOURCE_DIAGNOSTIC_AUTHENTICATION_MECHANISM_NAME_REQUIRED = 12,
    DLMS_SOURCE_DIAGNOSTIC_AUTHENTICATION_FAILURE = 13,
    DLMS_SOURCE_DIAGNOSTIC_AUTHENTICATION_REQUIRED = 14
  } DLMS_SOURCE_DIAGNOSTIC;

  // Defines whether or not the device has been assigned an address
  // since last power up of the device.
  typedef enum
  {
    // Not assigned an address yet.
    DLMS_ADDRESS_STATE_NONE,
    // Assigned an address either by manual setting, or by automated method.
    DLMS_ADDRESS_STATE_ASSIGNED
  } DLMS_ADDRESS_STATE;

  // Security policy Enforces authentication and/or encryption algorithm provided with security_suite.
  //Note! This enumeration values are used in Security Setup version 0.
  typedef enum
  {
    //No security is used.
    DLMS_SECURITY_POLICY_NOTHING = 0,
    /**
     All messages to be authenticated.
    */
    DLMS_SECURITY_POLICY_AUTHENTICATED = 1,
    /**
     All messages to be encrypted.
    */
    DLMS_SECURITY_POLICY_ENCRYPTED = 2,
    /**
     All messages to be authenticated and encrypted.
    */
    DLMS_SECURITY_POLICY_AUTHENTICATED_ENCRYPTED = 3
  } DLMS_SECURITY_POLICY;

  // Security policy Enforces authentication and/or encryption algorithm provided with security_suite.
  //Note! This enumeration values are used in Security Setup version 1.
  typedef enum
  {
    /**
     * Security is not used.
     */
    DLMS_SECURITY_POLICY1_NOTHING = 0,
    /**
     * Request is authenticated.
     */
     DLMS_SECURITY_POLICY1_AUTHENTICATED_REQUEST = 0x20,

     /**
      * Request is encrypted.
      */
      DLMS_SECURITY_POLICY1_ENCRYPTED_REQUEST = 0x10,

      /**
       * Request is digitally signed.
       */
       DLMS_SECURITY_POLICY1_DIGITALLY_SIGNED_REQUEST = 0x8,

       /**
        * Response is authenticated.
        */
        DLMS_SECURITY_POLICY1_AUTHENTICATED_RESPONSE = 0x4,

        /**
         * Response is encrypted.
         */
         DLMS_SECURITY_POLICY1_ENCRYPTED_RESPONSE = 0x2,

         /**
          * Response is digitally signed.
          */
          DLMS_SECURITY_POLICY1_DIGITALLY_SIGNED_RESPONSE = 0x1
  } DLMS_SECURITY_POLICY1;

  //Security suite Specifies authentication, encryption and key wrapping algorithm.
  typedef enum
  {
    /**
     AES-GCM-128 for authenticated encryption and AES-128 for key wrapping.
    */
    DLMS_SECURITY_SUITE_AES_GCM_128
  } DLMS_SECURITY_SUITE;

  typedef enum
  {
    DLMS_SERVICE_TYPE_TCP = 0,
    DLMS_SERVICE_TYPE_UDP = 1,
    DLMS_SERVICE_TYPE_FTP = 2,
    DLMS_SERVICE_TYPE_SMTP = 3,
    DLMS_SERVICE_TYPE_SMS = 4,
    DLMS_SERVICE_TYPE_HDLC = 5,
    DLMS_SERVICE_TYPE_M_BUS = 6,
    DLMS_SERVICE_TYPE_ZIG_BEE = 7
  } DLMS_SERVICE_TYPE;

  typedef enum
  {
    DLMS_MESSAGE_TYPE_COSEM_APDU = 0,
    DLMS_MESSAGE_TYPE_COSEM_APDU_XML = 1,
    DLMS_MESSAGE_TYPE_MANUFACTURER_SPECIFIC = 128
  } DLMS_MESSAGE_TYPE;

  typedef enum
  {
    DLMS_CLOCK_STATUS_OK = 0x0,
    DLMS_CLOCK_STATUS_INVALID_VALUE = 0x1,
    DLMS_CLOCK_STATUS_DOUBTFUL_VALUE = 0x2,
    DLMS_CLOCK_STATUS_DIFFERENT_CLOCK_BASE = 0x4,
    DLMS_CLOCK_STATUS_RESERVED1 = 0x8,
    DLMS_CLOCK_STATUS_RESERVED2 = 0x10,
    DLMS_CLOCK_STATUS_RESERVED3 = 0x20,
    DLMS_CLOCK_STATUS_RESERVED4 = 0x40,
    DLMS_CLOCK_STATUS_DAYLIGHT_SAVE_ACTIVE = 0x80,
    //Skip clock status on write.
    DLMS_CLOCK_STATUS_SKIP = 0xFF
  } DLMS_CLOCK_STATUS;

  /**
   * Used priority.
   */
  typedef enum
  {
    // Normal priority.
    DLMS_PRIORITY_NORMAL = 0,

    // High priority.
    DLMS_PRIORITY_HIGH = 1
  } DLMS_PRIORITY;

  typedef enum
  {
    DLMS_SERVICE_CLASS_UN_CONFIRMED = 0,
    DLMS_SERVICE_CLASS_CONFIRMED = 1
  } DLMS_SERVICE_CLASS;

  typedef enum
  {
    DLMS_OBJECT_TYPE_ALL = -1,
    DLMS_OBJECT_TYPE_NONE = 0,
    DLMS_OBJECT_TYPE_DATA = 1,
    DLMS_OBJECT_TYPE_REGISTER = 3,
    DLMS_OBJECT_TYPE_EXTENDED_REGISTER = 4,
    DLMS_OBJECT_TYPE_DEMAND_REGISTER = 5,
    DLMS_OBJECT_TYPE_REGISTER_ACTIVATION = 6,
    DLMS_OBJECT_TYPE_PROFILE_GENERIC = 7,
    DLMS_OBJECT_TYPE_CLOCK = 8,
    DLMS_OBJECT_TYPE_SCRIPT_TABLE = 9,
    DLMS_OBJECT_TYPE_SCHEDULE = 10,
    DLMS_OBJECT_TYPE_SPECIAL_DAYS_TABLE = 11,
    DLMS_OBJECT_TYPE_ASSOCIATION_SHORT_NAME = 12,
    DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME = 15,
    DLMS_OBJECT_TYPE_SAP_ASSIGNMENT = 17,
    DLMS_OBJECT_TYPE_IMAGE_TRANSFER = 18,
    DLMS_OBJECT_TYPE_IEC_LOCAL_PORT_SETUP = 19,
    DLMS_OBJECT_TYPE_ACTIVITY_CALENDAR = 20,
    DLMS_OBJECT_TYPE_REGISTER_MONITOR = 21,
    DLMS_OBJECT_TYPE_ACTION_SCHEDULE = 22,
    DLMS_OBJECT_TYPE_IEC_HDLC_SETUP = 23,
    DLMS_OBJECT_TYPE_IEC_TWISTED_PAIR_SETUP = 24,
    DLMS_OBJECT_TYPE_MBUS_SLAVE_PORT_SETUP = 25,
    DLMS_OBJECT_TYPE_UTILITY_TABLES = 26,
    DLMS_OBJECT_TYPE_MODEM_CONFIGURATION = 27,
    DLMS_OBJECT_TYPE_AUTO_ANSWER = 28,
    DLMS_OBJECT_TYPE_AUTO_CONNECT = 29,
    DLMS_OBJECT_TYPE_TCP_UDP_SETUP = 41,
    DLMS_OBJECT_TYPE_IP4_SETUP = 42,
    DLMS_OBJECT_TYPE_MAC_ADDRESS_SETUP = 43,
    DLMS_OBJECT_TYPE_PPP_SETUP = 44,
    DLMS_OBJECT_TYPE_GPRS_SETUP = 45,
    DLMS_OBJECT_TYPE_SMTP_SETUP = 46,
    DLMS_OBJECT_TYPE_REGISTER_TABLE = 61,
    DLMS_OBJECT_TYPE_STATUS_MAPPING = 63,
    DLMS_OBJECT_TYPE_SECURITY_SETUP = 64,
    DLMS_OBJECT_TYPE_DISCONNECT_CONTROL = 70,
    DLMS_OBJECT_TYPE_LIMITER = 71,
    DLMS_OBJECT_TYPE_MBUS_CLIENT = 72,
    DLMS_OBJECT_TYPE_PUSH_SETUP = 40,
    DLMS_OBJECT_TYPE_MESSAGE_HANDLER = 60,
    DLMS_OBJECT_TYPE_PARAMETER_MONITOR = 65,
    DLMS_OBJECT_TYPE_WIRELESS_MODE_Q_CHANNEL = 73,
    DLMS_OBJECT_TYPE_MBUS_MASTER_PORT_SETUP = 74,

    /**
     * Configure a ZigBee PRO device with information necessary to create or
     * join the network.
     */
     DLMS_OBJECT_TYPE_ZIG_BEE_SAS_STARTUP = 101,
     /**
      * Configure the behavior of a ZigBee PRO device on joining or loss of
      * connection to the network.
      */
      DLMS_OBJECT_TYPE_ZIG_BEE_SAS_JOIN = 102,
      /**
       * Configure the fragmentation feature of ZigBee PRO transport layer.
       */
       DLMS_OBJECT_TYPE_ZIG_BEE_SAS_APS_FRAGMENTATION = 103,

       DLMS_OBJECT_TYPE_ZIG_BEE_NETWORK_CONTROL = 104,

       DLMS_OBJECT_TYPE_DATA_PROTECTION = 30,
       DLMS_OBJECT_TYPE_ACCOUNT = 111,
       DLMS_OBJECT_TYPE_CREDIT = 112,
       DLMS_OBJECT_TYPE_CHARGE = 113,
       DLMS_OBJECT_TYPE_TOKEN_GATEWAY = 115,
  } DLMS_OBJECT_TYPE;

  typedef enum
  {
    HDLC_INFO_MAX_INFO_TX = 0x5,
    HDLC_INFO_MAX_INFO_RX = 0x6,
    HDLC_INFO_WINDOW_SIZE_TX = 0x7,
    HDLC_INFO_WINDOW_SIZE_RX = 0x8
  } HDLC_INFO;

  typedef enum
  {
    DLMS_DATA_TYPE_NONE = 0,
    DLMS_DATA_TYPE_BOOLEAN = 3,
    DLMS_DATA_TYPE_BIT_STRING = 4,
    DLMS_DATA_TYPE_INT32 = 5,
    DLMS_DATA_TYPE_UINT32 = 6,
    DLMS_DATA_TYPE_OCTET_STRING = 9,
    DLMS_DATA_TYPE_STRING = 10,
    DLMS_DATA_TYPE_BINARY_CODED_DESIMAL = 13,
    DLMS_DATA_TYPE_STRING_UTF8 = 12,
    DLMS_DATA_TYPE_INT8 = 15,
    DLMS_DATA_TYPE_INT16 = 16,
    DLMS_DATA_TYPE_UINT8 = 17,
    DLMS_DATA_TYPE_UINT16 = 18,
    DLMS_DATA_TYPE_INT64 = 20,
    DLMS_DATA_TYPE_UINT64 = 21,
    DLMS_DATA_TYPE_ENUM = 22,
    DLMS_DATA_TYPE_FLOAT32 = 23,
    DLMS_DATA_TYPE_FLOAT64 = 24,
    DLMS_DATA_TYPE_DATETIME = 25,
    DLMS_DATA_TYPE_DATE = 26,
    DLMS_DATA_TYPE_TIME = 27,
    DLMS_DATA_TYPE_ARRAY = 1,
    DLMS_DATA_TYPE_STRUCTURE = 2,
    DLMS_DATA_TYPE_COMPACT_ARRAY = 19
  } DLMS_DATA_TYPE;

  typedef enum
  {
    /// <summary>
    /// No access.
    /// </summary>
    DLMS_ACCESS_MODE_NONE = 0,
    /// <summary>
    /// The client is allowed only reading from the server.
    /// </summary>
    DLMS_ACCESS_MODE_READ = 1,
    /// <summary>
    /// The client is allowed only writing to the server.
    /// </summary>
    DLMS_ACCESS_MODE_WRITE = 2,
    /// <summary>
    /// The client is allowed both reading from the server and writing to it.
    /// </summary>
    DLMS_ACCESS_MODE_READ_WRITE = 3,
    DLMS_ACCESS_MODE_AUTHENTICATED_READ = 4,
    DLMS_ACCESS_MODE_AUTHENTICATED_WRITE = 5,
    DLMS_ACCESS_MODE_AUTHENTICATED_READ_WRITE = 6
  } DLMS_ACCESS_MODE;

  typedef enum
  {
    /// <summary>
    /// No access.
    /// </summary>
    DLMS_METHOD_ACCESS_MODE_NONE = 0,
    /// <summary>
    /// Access.
    /// </summary>
    DLMS_METHOD_ACCESS_MODE_ACCESS = 1,
    /// <summary>
    /// Authenticated Access.
    /// </summary>
    DLMS_METHOD_ACCESS_MODE_AUTHENTICATED_ACCESS = 2
  } DLMS_METHOD_ACCESS_MODE;

  typedef enum
  {
    DLMS_ASSOCIATION_STATUS_NON_ASSOCIATED = 0,
    DLMS_ASSOCIATION_STATUS_ASSOCIATION_PENDING = 1,
    DLMS_ASSOCIATION_STATUS_ASSOCIATED = 2
  } DLMS_ASSOCIATION_STATUS;

  /**
   * Enumerates Action request types.
   */
  typedef enum
  {
    /**
     * Normal action.
     */
    DLMS_ACTION_COMMAND_TYPE_NORMAL = 1,
    /**
     * Next block.
     */
     DLMS_ACTION_COMMAND_TYPE_NEXT_BLOCK = 2,
     /**
      * Action with list.
      */
      DLMS_ACTION_COMMAND_TYPE_WITH_LIST = 3,
      /**
       * Action with first block.
       */
       DLMS_ACTION_COMMAND_TYPE_WITH_FIRST_BLOCK = 4,
       /**
        * Action with list and first block.
        */
        DLMS_ACTION_COMMAND_TYPE_WITH_LIST_AND_FIRST_BLOCK = 5,
        /**
         * Action with block.
         */
         DLMS_ACTION_COMMAND_TYPE_WITH_BLOCK = 6
  } DLMS_ACTION_COMMAND_TYPE;

  typedef enum
  {
    /**
     If this option is present, the device shall be allowed to send security,
     compartmentation, handling restrictions and TCC (closed user group)
     parameters within its IP Datagrams. The value of the IP-Option-
     Length Field must be 11, and the IP-Option-Data shall contain the
     value of the Security, Compartments, Handling Restrictions and
     Transmission Control Code values, as specified in STD0005 / RFC791.
    */
    DLMS_IP_OPTION_TYPE_SECURITY = 0x82,
    /**
     If this option is present, the device shall supply routing information to be
     used by the gateways in forwarding the datagram to the destination, and to
     record the route information.
     The IP-Option-length and IP-Option-Data values are specified in STD0005 / RFC 791.
    */
    DLMS_IP_OPTION_TYPE_LOOSE_SOURCE_AND_RECORD_ROUTE = 0x83,
    /**
     If this option is present, the device shall supply routing information to be
     used by the gateways in forwarding the datagram to the destination, and to
     record the route information.
     The IP-Option-length and IP-Option-Data values are specified in STD0005 / RFC 791.
    */
    DLMS_IP_OPTION_TYPE_STRICT_SOURCE_AND_RECORD_ROUTE = 0x89,
    /**
     If this option is present, the device shall as well:
     send originated IP Datagrams with that option, providing means
     to record the route of these Datagrams;
     as a router, send routed IP Datagrams with the route option
     adjusted according to this option.
     The IP-Option-length and IP-Option-Data values are specified in
     STD0005 / RFC 791.
    */
    DLMS_IP_OPTION_TYPE_RECORD_ROUTE = 0x07,
    /**
     If this option is present, the device shall as well:
     send originated IP Datagrams with that option, providing means
     to time-stamp the datagram in the route to its destination;
     as a router, send routed IP Datagrams with the time-stamp option
     adjusted according to this option.
     The IP-Option-length and IP-Option-Data values are specified in STD0005 / RFC 791.
    */
    DLMS_IP_OPTION_TYPE_INTERNET_TIMESTAMP = 0x44
  } DLMS_IP_OPTION_TYPE;

  typedef enum
  {
    /**
     Size of execution_time = 1. Wildcard in date allowed.
    */
    DLMS_SINGLE_ACTION_SCHEDULE_TYPE1 = 0,
    /**
     Size of execution_time = n.
     All time values are the same, wildcards in date not allowed.
    */
    DLMS_SINGLE_ACTION_SCHEDULE_TYPE2 = 1,
    /**
     Size of execution_time = n.
     All time values are the same, wildcards in date are allowed,
    */
    DLMS_SINGLE_ACTION_SCHEDULE_TYPE3 = 2,
    /**
     Size of execution_time = n.
     Time values may be different, wildcards in date not allowed,
    */
    DLMS_SINGLE_ACTION_SCHEDULE_TYPE4 = 3,
    /**
     Size of execution_time = n.
     Time values may be different, wildcards in date are allowed
    */
    DLMS_SINGLE_ACTION_SCHEDULE_TYPE5 = 4
  } DLMS_SINGLE_ACTION_SCHEDULE_TYPE;

  typedef enum
  {
    DLMS_IMAGE_TRANSFER_STATUS_NOT_INITIATED,
    DLMS_IMAGE_TRANSFER_STATUS_INITIATED,
    DLMS_IMAGE_TRANSFER_STATUS_VERIFICATION_INITIATED,
    DLMS_IMAGE_TRANSFER_STATUS_VERIFICATION_SUCCESSFUL,
    DLMS_IMAGE_TRANSFER_STATUS_VERIFICATION_FAILED,
    DLMS_IMAGE_TRANSFER_STATUS_ACTIVATION_INITIATED,
    DLMS_IMAGE_TRANSFER_STATUS_ACTIVATION_SUCCESSFUL,
    DLMS_IMAGE_TRANSFER_STATUS_ACTIVATION_FAILED
  } DLMS_IMAGE_TRANSFER_STATUS;

  typedef enum
  {
    /**
     * Transport security is not used.
     */
    DLMS_SECURITY_NONE = 0,

    /**
     * Authentication security is used.
     */
     DLMS_SECURITY_AUTHENTICATION = 0x10,

     /**
      * Encryption security is used.
      */
      DLMS_SECURITY_ENCRYPTION = 0x20,

      /**
       * Authentication and Encryption security are used.
       */
       DLMS_SECURITY_AUTHENTICATION_ENCRYPTION = 0x30,
  } DLMS_SECURITY;

  typedef enum
  {
    DLMS_COUNT_TYPE_TAG = 0x1,
    DLMS_COUNT_TYPE_DATA = 2,
    DLMS_COUNT_TYPE_PACKET = 3
  } DLMS_COUNT_TYPE;

  /**
  * HDLC frame types.
  */
  typedef enum
  {
    /**
    * I-Frame. Information frames are used to transport user data.
    */
    HDLC_FRAME_TYPE_I_FRAME = 0x0,
    /**
     * S-frame. Supervisory Frames are used for flow and error control.
     * Rejected, RNR and RR.
     */
     HDLC_FRAME_TYPE_S_FRAME = 0x1,
     /**
      * U-frame. Unnumbered frames are used for link management. Example SNRM and
      * UA.
      */
      HDLC_FRAME_TYPE_U_FRAME = 0x3
  } HDLC_FRAME_TYPE;

  /**
  *BER encoding enumeration values.
  */
  typedef enum
  {
    /**
    * End of Content.
    */
    BER_TYPE_EOC = 0x00,
    /**
    * Boolean.
    */
    BER_TYPE_BOOLEAN = 0x1,
    /**
    * Integer.
    */
    BER_TYPE_INTEGER = 0x2,
    /**
    * Bit String.
    */
    BER_TYPE_BIT_STRING = 0x3,
    /**
    * Octet string.
    */
    BER_TYPE_OCTET_STRING = 0x4,
    /**
    * Null value.
    */
    BER_TYPE_NULL = 0x5,
    /**
    * Object identifier.
    */
    BER_TYPE_OBJECT_IDENTIFIER = 0x6,
    /**
    * Object Descriptor.
    */
    BER_TYPE_OBJECT_DESCRIPTOR = 7,
    /**
    * External
    */
    BER_TYPE_EXTERNAL = 8,
    /**
    * Real (float).
    */
    BER_TYPE_REAL = 9,
    /**
    * Enumerated.
    */
    BER_TYPE_ENUMERATED = 10,
    /**
    * Utf8 String.
    */
    BER_TYPE_UTF8_STRING = 12,
    /**
    * Numeric string.
    */
    BER_TYPE_NUMERIC_STRING = 18,
    /**
    * Printable string.
    */
    BER_TYPE_PRINTABLE_STRING = 19,
    /**
    * Teletex string.
    */
    BER_TYPE_TELETEX_STRING = 20,
    /**
    * Videotex string.
    */
    BER_TYPE_VIDEOTEX_STRING = 21,
    /**
    * Ia5 string
    */
    BER_TYPE_IA5_STRING = 22,
    /**
    * Utc time.
    */
    BER_TYPE_UTC_TIME = 23,
    /**
    * Generalized time.
    */
    BER_TYPE_GENERALIZED_TIME = 24,
    /**
    * Graphic string.
    */
    BER_TYPE_GRAPHIC_STRING = 25,
    /**
    * Visible string.
    */
    BER_TYPE_VISIBLE_STRING = 26,
    /**
    * General string.
    */
    BER_TYPE_GENERAL_STRING = 27,
    /**
    * Universal string.
    */
    BER_TYPE_UNIVERSAL_STRING = 28,
    /**
    * Bmp string.
    */
    BER_TYPE_BMP_STRING = 30,
    /**
    * Application class.
    */
    BER_TYPE_APPLICATION = 0x40,
    /**
    * Context class.
    */
    BER_TYPE_CONTEXT = 0x80,
    /**
    * Private class.
    */
    BER_TYPE_PRIVATE = 0xc0,
    /**
    * Constructed.
    */
    BER_TYPE_CONSTRUCTED = 0x20
  } BER_TYPE;

  /**
   * APDU types.
   */
  typedef enum
  {
    /**
     * IMPLICIT BIT STRING {version1  = 0} DEFAULT {version1}
     */
    PDU_TYPE_PROTOCOL_VERSION = 0,

    /**
     * Application-context-name
     */
     PDU_TYPE_APPLICATION_CONTEXT_NAME = 1,

     /**
      * AP-title OPTIONAL
      */
      PDU_TYPE_CALLED_AP_TITLE = 2,

      /**
       * AE-qualifier OPTIONAL.
       */
       PDU_TYPE_CALLED_AE_QUALIFIER = 3,

       /**
        * AP-invocation-identifier OPTIONAL.
        */
        PDU_TYPE_CALLED_AP_INVOCATION_ID = 4,

        /**
         * AE-invocation-identifier OPTIONAL
         */
         PDU_TYPE_CALLED_AE_INVOCATION_ID = 5,

         /**
          * AP-title OPTIONAL
          */
          PDU_TYPE_CALLING_AP_TITLE = 6,

          /**
           * AE-qualifier OPTIONAL
           */
           PDU_TYPE_CALLING_AE_QUALIFIER = 7,

           /**
            * AP-invocation-identifier OPTIONAL
            */
            PDU_TYPE_CALLING_AP_INVOCATION_ID = 8,

            /**
             * AE-invocation-identifier OPTIONAL
             */
             PDU_TYPE_CALLING_AE_INVOCATION_ID = 9,

             /**
              * The following field shall not be present if only the kernel is used.
              */
              PDU_TYPE_SENDER_ACSE_REQUIREMENTS = 10,

              /**
               * The following field shall only be present if the authentication
               * functional unit is selected.
               */
               PDU_TYPE_MECHANISM_NAME = 11,

               /**
                * The following field shall only be present if the authentication
                * functional unit is selected.
                */
                PDU_TYPE_CALLING_AUTHENTICATION_VALUE = 12,

                /**
                 * Implementation-data.
                 */
                 PDU_TYPE_IMPLEMENTATION_INFORMATION = 29,

                 /**
                  * Association-information OPTIONAL
                  */
                  PDU_TYPE_USER_INFORMATION = 30
  } PDU_TYPE;

  /**
   * HDLC control frame types.
   */
  typedef enum
  {
    /**
     * Receive is ready.
     */
    HDLC_CONTROL_FRAME_RECEIVE_READY = 0,
    /**
     * Receive is not ready.
     */
     HDLC_CONTROL_FRAME_RECEIVE_NOT_READY = 0x1,
     /**
      * Frame is rejected.
      */
      HDLC_CONTROL_FRAME_REJECT = 2,
      /**
       * Frame is selective rejected. Not all meters support this.
       */
       HDLC_CONTROL_FRAME_SELECTIVE_REJECT = 3
  } HDLC_CONTROL_FRAME;

  /**
   * Authentication enumerates the authentication levels.
   */
  typedef enum
  {
    /**
     * No authentication is used.
     */
    DLMS_AUTHENTICATION_NONE,

    /**
     * Low authentication is used.
     */
     DLMS_AUTHENTICATION_LOW,

     /**
      * High authentication is used. Because DLMS/COSEM specification does not
      * specify details of the HLS mechanism we have implemented Indian standard.
      * Texas Instruments also uses this.
      */
      DLMS_AUTHENTICATION_HIGH,

      /**
       * High authentication is used. Password is hashed with MD5.
       */
       DLMS_AUTHENTICATION_HIGH_MD5,

       /**
        * High authentication is used. Password is hashed with SHA1.
        */
        DLMS_AUTHENTICATION_HIGH_SHA1,

        /**
         * High authentication is used. Password is hashed with GMAC.
         */
         DLMS_AUTHENTICATION_HIGH_GMAC
  } DLMS_AUTHENTICATION;

  //Interface type that is used.
  typedef enum
  {
    //General
    DLMS_INTERFACE_TYPE_HDLC = 0,
    // IEC 62056-47 COSEM transport layers for IPv4 networks
    DLMS_INTERFACE_TYPE_WRAPPER = 0x1
  } DLMS_INTERFACE_TYPE;

  typedef enum
  {
    DLMS_DATA_REQUEST_TYPES_NONE = 0x0,
    DLMS_DATA_REQUEST_TYPES_FRAME = 0x1,
    DLMS_DATA_REQUEST_TYPES_BLOCK = 0x2,
    DLMS_DATA_REQUEST_TYPES_LAST_BLOCK = 0x3,
  } DLMS_DATA_REQUEST_TYPES;

  /**
   * DLMS commands.
   */
  typedef enum
  {
    /**
    * No command to execute.
    */
    DLMS_COMMAND_NONE = 0,

    /**
     * Initiate request.
     */
     DLMS_COMMAND_INITIATE_REQUEST = 0x1,

     /**
      * Initiate response.
      */
      DLMS_COMMAND_INITIATE_RESPONSE = 0x8,

      /**
       * Read request.
       */
       DLMS_COMMAND_READ_REQUEST = 0x5,

       /**
        * Read response.
        */
        DLMS_COMMAND_READ_RESPONSE = 0xC,

        /**
         * Write request.
         */
         DLMS_COMMAND_WRITE_REQUEST = 0x6,

         /**
          * Write response.
          */
          DLMS_COMMAND_WRITE_RESPONSE = 0xD,

          /**
           * Get request.
           */
           DLMS_COMMAND_GET_REQUEST = 0xC0,

           /**
            * Get response.
            */
            DLMS_COMMAND_GET_RESPONSE = 0xC4,

            /**
             * Set request.
             */
             DLMS_COMMAND_SET_REQUEST = 0xC1,

             /**
              * Set response.
              */
              DLMS_COMMAND_SET_RESPONSE = 0xC5,

              /**
               * Action request.
               */
               DLMS_COMMAND_METHOD_REQUEST = 0xC3,

               /**
                * Action response.
                */
                DLMS_COMMAND_METHOD_RESPONSE = 0xC7,

                /**
                 * Command rejected.
                 */
                 DLMS_COMMAND_REJECTED = 0x97,

                 /**
                  * SNRM request.
                  */
                  DLMS_COMMAND_SNRM = 0x93,

                  /**
                   * UA request.
                   */
                   DLMS_COMMAND_UA = 0x73,
                   DLMS_COMMAND_DM = 0x1F,
                   /**
                    * AARQ request.
                    */
                    DLMS_COMMAND_AARQ = 0x60,

                    /**
                     * AARE request.
                     */
                     DLMS_COMMAND_AARE = 0x61,

                     /**
                      * Disconnect request for HDLC framing.
                      */
                      DLMS_COMMAND_DISC = 0x53,

                      /**
                       * Disconnect request.
                       */
                       DLMS_COMMAND_DISCONNECT_REQUEST = 0x62,
                       DLMS_COMMAND_FRMR = 0x97,

                       /**
                        * Disconnect response.
                        */
                        DLMS_COMMAND_DISCONNECT_RESPONSE = 0x63,

                        /**
                        * Confirmed Service Error.
                        */
                        DLMS_COMMAND_CONFIRMED_SERVICE_ERROR = 0x0E,

                        /**
                         * Exception Response.
                         */
                         DLMS_COMMAND_EXCEPTION_RESPONSE = 0xD8,

                         /**
                          * General Block Transfer.
                          */
                          DLMS_COMMAND_GENERAL_BLOCK_TRANSFER = 0xE0,

                          /**
                           * Data Notification request.
                           */
                           DLMS_COMMAND_DATA_NOTIFICATION = 0x0F,

                           /**
                            * Glo get request.
                            */
                            DLMS_COMMAND_GLO_GET_REQUEST = 0xC8,

                            /**
                             * Glo get response.
                             */
                             DLMS_COMMAND_GLO_GET_RESPONSE = 0xCC,

                             /**
                              * Glo set request.
                              */
                              DLMS_COMMAND_GLO_SET_REQUEST = 0xC9,

                              /**
                               * Glo set response.
                               */
                               DLMS_COMMAND_GLO_SET_RESPONSE = 0xCD,

                               /**
                                * Glo general ciphering.
                                */
                                DLMS_COMMAND_GLO_GENERAL_CIPHERING = 0xDB,

                                /**
                                 * Glo event notification request.
                                 */
                                 DLMS_COMMAND_GLO_EVENT_NOTIFICATION_REQUEST = 0xCA,

                                 /**
                                  * Glo method request.
                                  */
                                  DLMS_COMMAND_GLO_METHOD_REQUEST = 0xCB,

                                  /**
                                   * Glo method response.
                                   */
                                   DLMS_COMMAND_GLO_METHOD_RESPONSE = 0xCF,

                                   /**
                                    * Glo Initiate request.
                                    */
                                    DLMS_COMMAND_GLO_INITIATE_REQUEST = 0x21,
                                    /**
                                     * Glo read request.
                                     */
                                     DLMS_COMMAND_GLO_READ_REQUEST = 37,

                                     /**
                                      * Glo write request.
                                      */
                                      DLMS_COMMAND_GLO_WRITE_REQUEST = 38,
                                      /**
                                       * Glo Initiate response.
                                       */
                                       DLMS_COMMAND_GLO_INITIATE_RESPONSE = 40,
                                       /**
                                        * Glo read response.
                                        */
                                        DLMS_COMMAND_GLO_READ_RESPONSE = 44,
                                        /**
                                         * Glo write response.
                                         */
                                         DLMS_COMMAND_GLO_WRITE_RESPONSE = 45,
                                         ////////////////////////////////////////
                                         /**
                                          * Ded initiate Request.
                                          */
                                          DLMS_COMMAND_DED_INITIATE_REQUEST = 65,

                                          /**
                                           * Ded Read Request.
                                           */
                                           DLMS_COMMAND_DED_READ_REQUEST = 69,

                                           /**
                                            * Ded Write Request.
                                            */
                                            DLMS_COMMAND_DED_WRITE_REQUEST = 70,

                                            /**
                                             * Ded Initiate Response.
                                             */
                                             DLMS_COMMAND_DED_INITIATE_RESPONSE = 72,

                                             /**
                                              * Ded Read Response.
                                              */
                                              DLMS_COMMAND_DED_READ_RESPONSE = 76,

                                              /**
                                               * Ded Write Response.
                                               */
                                               DLMS_COMMAND_DED_WRITE_RESPONSE = 77,

                                               /**
                                                * Ded Confirmed ServiceError.
                                                */
                                                DLMS_COMMAND_DED_CONFIRMED_SERVICE_ERROR = 78,

                                                /**
                                                 * Ded Unconfirmed Write Request.
                                                 */
                                                 DLMS_COMMAND_DED_UNCONFIRMED_WRITE_REQUEST = 86,

                                                 /**
                                                  * Ded Information Report Request.
                                                  */
                                                  DLMS_COMMAND_DED_INFORMATION_REPORT_REQUEST = 88,

                                                  /**
                                                   * Ded Get Request.
                                                   */
                                                   DLMS_COMMAND_DED_GET_REQUEST = 208,

                                                   /**
                                                    * Ded Set Request.
                                                    */
                                                    DLMS_COMMAND_DED_SET_REQUEST = 209,

                                                    /**
                                                     * Ded Event Notification Request.
                                                     */
                                                     DLMS_COMMAND_DED_EVENT_NOTIFICATION_REQUEST = 210,

                                                     /**
                                                      * Ded Action Request.
                                                      */
                                                      DLMS_COMMAND_DED_ACTION_REQUEST = 211,

                                                      /**
                                                       * Ded Get Response.
                                                       */
                                                       DLMS_COMMAND_DED_GET_RESPONSE = 212,

                                                       /**
                                                        * Ded Set Response.
                                                        */
                                                        DLMS_COMMAND_DED_SET_RESPONSE = 213,

                                                        /**
                                                         * Ded Action Response.
                                                         */
                                                         DLMS_COMMAND_DED_ACTION_RESPONSE = 215,

                                                         /**
                                                          * General Ded Ciphering.
                                                          */
                                                          DLMS_COMMAND_GENERAL_DED_CIPHERING = 220,

                                                          DLMS_RR_REQUEST = 0xFE
  } DLMS_COMMAND;

  /**
  * Confirmed service error tells when error has occurred.
  */
  typedef enum
  {
    /**
     * Error has occurred on initialize.
     */
    DLMS_CONFIRMED_SERVICE_ERROR_INITIATE_ERROR = 1,
    /**
     * Error has occurred on read.
     */
     DLMS_CONFIRMED_SERVICE_ERROR_READ = 5,
     /**
      * Error has occurred on write.
      */
      DLMS_CONFIRMED_SERVICE_ERROR_WRITE = 6
  } DLMS_CONFIRMED_SERVICE_ERROR;

  /**
   * Access describes access errors.
   */
  typedef enum
  {
    /**
     * Other error.
     */
    DLMS_ACCESS_OTHER = 0,
    /**
     * Scope of access violated.
     */
     DLMS_ACCESS_SCOPE_OF_ACCESS_VIOLATED = 1,
     /**
      * Object access is invalid.
      */
      DLMS_ACCESS_OBJECT_ACCESS_INVALID = 2,
      /**
       * Hardware fault.
       */
       DLMS_ACCESS_HARDWARE_FAULT = 3,
       /**
        * Object is unavailable.
        */
        DLMS_ACCESS_OBJECT_UNAVAILABLE = 4
  } DLMS_ACCESS;

  /**
   * Application reference describes application errors.
   */
  typedef enum
  {
    /**
     * Other error is occurred.
     */
    DLMS_APPLICATION_REFERENCE_OTHER = 0,
    /**
     * Time elapsed.
     */
     DLMS_APPLICATION_REFERENCE_TIME_ELAPSED = 1,
     /**
      * Application unreachable.
      */
      DLMS_APPLICATION_REFERENCE_APPLICATION_UNREACHABLE = 2,
      /**
       * Application reference is invalid.
       */
       DLMS_APPLICATION_REFERENCE_APPLICATION_REFERENCE_INVALID = 3,
       /**
        * Application context unsupported.
        */
        DLMS_APPLICATION_REFERENCE_APPLICATION_CONTEXT_UNSUPPORTED = 4,
        /**
         * Provider communication error.
         */
         DLMS_APPLICATION_REFERENCE_PROVIDER_COMMUNICATION_ERROR = 5,
         /**
          * Deciphering error.
          */
          DLMS_APPLICATION_REFERENCE_DECIPHERING_ERROR = 6
  } DLMS_APPLICATION_REFERENCE;

  /**
   * Definition describes definition errors.
   */
  typedef enum
  {
    /**
     * Other error.
     */
    DLMS_DEFINITION_OTHER = 0,
    /**
     * Object is Undefined.
     */
     DLMS_DEFINITION_OBJECT_UNDEFINED = 1,
     /**
      * Object class inconsistent.
      */
      DLMS_DEFINITION_OBJECT_CLASS_INCONSISTENT = 2,
      /**
       * Object attribute inconsistent.
       */
       DLMS_DEFINITION_OBJECT_ATTRIBUTE_INCONSISTENT = 3
  } DLMS_DEFINITION;

  /**
   * DLMS service errors.
   *
   * @author Gurux Ltd.
   */
  typedef enum
  {
    /**
     * Operation is not possible
     */
    DLMS_EXCEPTION_SERVICE_ERROR_OPERATION_NOT_POSSIBLE = 1,
    /**
     * Service is not supported.
     */
     DLMS_EXCEPTION_SERVICE_ERROR_SERVICE_NOT_SUPPORTED = 2,
     /**
      * Other reason.
      */
      DLMS_EXCEPTION_SERVICE_ERROR_OTHER_REASON = 3
  } DLMS_EXCEPTION_SERVICE_ERROR;

  /**
   * Hardware resource describes hardware errors.
   */
  typedef enum
  {
    /**
     * Other hardware resource error.
     */
    DLMS_HARDWARE_RESOURCE_OTHER = 0,
    /**
     * Memory is unavailable.
     */
     DLMS_HARDWARE_RESOURCE_MEMORY_UNAVAILABLE = 1,
     /**
      * Processor resource is unavailable.
      */
      DLMS_HARDWARE_RESOURCE_PROCESSOR_RESOURCE_UNAVAILABLE = 2,
      /**
       * Mass storage is unavailable.
       */
       DLMS_HARDWARE_RESOURCE_MASS_STORAGE_UNAVAILABLE = 3,
       /**
        * Other resource is unavailable.
        */
        DLMS_HARDWARE_RESOURCE_OTHER_RESOURCE_UNAVAILABLE = 4
  } DLMS_HARDWARE_RESOURCE;

  /**
   * Initiate describes onitiate errors.
   */
  typedef enum
  {
    /**
     * Other error.
     */
    DLMS_INITIATE_OTHER = 0,
    /**
     * Dlms version is too low.
     */
     DLMS_INITIATE_DLMS_VERSION_TOO_LOW = 1,
     /**
      * Incompatible conformance.
      */
      DLMS_INITIATE_INCOMPATIBLE_CONFORMANCE = 2,
      /**
       * Pdu size is too short.
       */
       DLMS_INITIATE_PDU_SIZE_TOOSHORT = 3,
       /**
        * Refused by the VDE handler.
        */
        DLMS_INITIATE_REFUSED_BY_THE_VDE_HANDLER = 4
  } DLMS_INITIATE;

  /**
   * LoadDataSet describes load dataset errors.
   */
  typedef enum
  {
    /**
     * Other error.
     */
    DLMS_LOAD_DATASET_OTHER = 0,
    /**
     * Primitive out of sequence.
     */
     DLMS_LOAD_DATASET_PRIMITIVE_OUT_OF_SEQUENCE = 1,
     /**
      * Not loadable.
      */
      DLMS_LOAD_DATASET_NOT_LOADABLE = 2,
      /**
       * Dataset size is too large.
       */
       DLMS_LOAD_DATASET_SIZE_TOO_LARGE = 3,
       /**
        * Not awaited segment.
        */
        DLMS_LOAD_DATASET_NOT_AWAITED_SEGMENT = 4,
        /**
         * Interpretation failure.
         */
         DLMS_LOAD_DATASET_INTERPRETATION_FAILURE = 5,
         /**
          * Storage failure.
          */
          DLMS_LOAD_DATASET_STORAGE_FAILURE = 6,
          /**
           * Dataset not ready.
           */
           DLMS_LOAD_DATASET_NOT_READY = 7
  } DLMS_LOAD_DATASET;

  /**
   * Task describes load task errors.
   */
  typedef enum
  {
    /**
     * Other error.
     */
    DLMS_TASK_OTHER = 0,
    /**
     * No remote control.
     */
     DLMS_TASK_NO_REMOTE_CONTROL = 1,
     /**
      * Ti is stopped.
      */
      DLMS_TASK_TI_STOPPED = 2,
      /**
       * TI is running.
       */
       DLMS_TASK_TI_RUNNIN = 3,
       /**
        * TI is unusable.
        */
        DLMS_TASK_TI_UNUSABLE = 4
  } DLMS_TASK;

  /**
   * VdeState error describes Vde state errors.
   */
  typedef enum
  {
    /**
     * Other error.
     */
    DLMS_VDE_STATE_ERROR_OTHER = 0,
    /**
     * No DLMS context.
     */
     DLMS_VDE_STATE_ERROR_NO_DLMS_CONTEXT = 1,
     /**
      * Loading dataset error.
      */
      DLMS_VDE_STATE_ERROR_LOADING_DATASET = 2,
      /**
       * Status No change.
       */
       DLMS_VDE_STATE_ERROR_STATUS_NO_CHANGE = 3,
       /**
        * Status Inoperable.
        */
        DLMS_VDE_STATE_ERROR_STATUS_INOPERABLE = 4
  } DLMS_VDE_STATE_ERROR;

  /**
   * Enumerates Get request and response types.
   */
  typedef enum
  {
    /**
     * Normal Get.
     */
    DLMS_GET_COMMAND_TYPE_NORMAL = 1,
    /**
     * Next data block.
     */
     DLMS_GET_COMMAND_TYPE_NEXT_DATA_BLOCK = 2,
     /**
      * Get request with list.
      */
      DLMS_GET_COMMAND_TYPE_WITH_LIST = 3
  } DLMS_GET_COMMAND_TYPE;

  /**
   * Service describes service errors.
   */
  typedef enum
  {
    /**
     * Other error.
     */
    DLMS_SERVICE_OTHER = 0,
    /**
     * PDU size is wrong.
     */
     DLMS_SERVICE_PDU_SIZE = 1,
     /**
      * Service is unsupported.
      */
      DLMS_SERVICE_UNSUPPORTED = 2
  } DLMS_SERVICE;

  /**
   * ServiceError enumerates service errors.
   */
  typedef enum
  {
    /**
     * Application error.
     */
    DLMS_SERVICE_ERROR_APPLICATION_REFERENCE = 0,
    /**
     * Hardware error.
     */
     DLMS_SERVICE_ERROR_HARDWARE_RESOURCE = 1,
     /**
      * Vde state error.
      */
      DLMS_SERVICE_ERROR_VDE_STATE_ERROR = 2,
      /**
       * Service error.
       */
       DLMS_SERVICE_ERROR_SERVICE = 3,
       /**
        * Definition error.
        */
        DLMS_SERVICE_ERROR_DEFINITION = 4,
        /**
         * Access error.
         */
         DLMS_SERVICE_ERROR_ACCESS = 5,
         /**
          * Initiate error.
          */
          DLMS_SERVICE_ERROR_INITIATE = 6,
          /**
           * LoadDataSet error.
           */
           DLMS_SERVICE_ERROR_LOAD_DATASET = 7,
           /**
            * Task error.
            */
            DLMS_SERVICE_ERROR_TAS = 8,
            /**
             * Other error describes manufacturer specific error code.
             */
             DLMS_SERVICE_ERROR_OTHER_ERROR = 9
  } DLMS_SERVICE_ERROR;

  /**
   * Enumerates Set request types.
   */
  typedef enum
  {
    /**
     * Normal Set.
     */
    DLMS_SET_COMMAND_TYPE_NORMAL = 1,
    /**
     * Set with first data block.
     */
     DLMS_SET_COMMAND_TYPE_FIRST_DATABLOCK = 2,
     /**
      * Set with data block.
      */
      DLMS_SET_COMMAND_TYPE_WITH_DATABLOCK = 3,
      /**
       * Set with list .
       */
       DLMS_SET_COMMAND_TYPE_WITH_LIST = 4,
       /**
        * Set with list and first data block.
        */
        DLMS_SET_COMMAND_TYPE_WITH_LIST_AND_WITH_FIRST_DATA_BLOCK = 5
  } DLMS_SET_COMMAND_TYPE;

  /**
   * Enumerates single read response types.
   */
  typedef enum
  {
    /**
     * Normal data.
     */
    DLMS_SINGLE_READ_RESPONSE_DATA = 0,
    /**
     * Error has occurred on read.
     */
     DLMS_SINGLE_READ_RESPONSE_DATA_ACCESS_ERROR = 1,
     /**
      * Return data as blocks.
      */
      DLMS_SINGLE_READ_RESPONSE_DATA_BLOCK_RESULT = 2,
      /**
       * Return block number.
       */
       DLMS_SINGLE_READ_RESPONSE_BLOCK_NUMBER = 3
  } DLMS_SINGLE_READ_RESPONSE;

  /**
   * Enumerates single write response types.
   */
  typedef enum
  {
    /**
     * Write succeeded.
     */
    DLMS_SINGLE_WRITE_RESPONSE_SUCCESS = 0,
    /**
     * Write error has occurred.
     */
     DLMS_SINGLE_WRITE_RESPONSE_DATA_ACCESS_ERROR = 1,
     /**
      * Get next block.
      */
      DLMS_SINGLE_WRITE_RESPONSE_BLOCK_NUMBER = 2
  } DLMS_SINGLE_WRITE_RESPONSE;

  /**
   * Enumerates how data is access on read or write.
   */
  typedef enum
  {
    /**
     * Read data using SN.
     */
    DLMS_VARIABLE_ACCESS_SPECIFICATION_VARIABLE_NAME = 2,
    /**
     * Get data using parameterized access.
     */
     DLMS_VARIABLE_ACCESS_SPECIFICATION_PARAMETERISED_ACCESS = 4,
     /**
      * Get next block.
      */
      DLMS_VARIABLE_ACCESS_SPECIFICATION_BLOCK_NUMBER_ACCESS = 5,
      /**
       * Read data as blocks.
       */
       DLMS_VARIABLE_ACCESS_SPECIFICATION_READ_DATA_BLOCK_ACCESS = 6,
       /**
        * Write data as blocks.
        */
        DLMS_VARIABLE_ACCESS_SPECIFICATION_WRITE_DATA_BLOCK_ACCESS = 7
  } DLMS_VARIABLE_ACCESS_SPECIFICATION;

#endif //ENUMS_H
