Copyright (c) 2016, Kamil Jastrzab
All rights reserved.

# Image-Encrypter
Image Encrypter is free stenography application that enables to hide message picture inside a storage picture.
Application allows user to pick number of bits that will carry the message into storage, and if the message should be hidden grey or in colour.
What application does is to divide colour bytes into segments most signing bits are for storage picture, least signing bits are for message picture.
What is important that hiddden messages can be decrypted if picture was not compressed with loses.


Picture on the left is storage picture - please choose picture with lot of noise (not uniform colors).
Picture on the right is message that will be hidden inside storage picture. After encryption quality of this image will drop according
ly to number of bits picked.

In order to Encrypt: Pick Storage Image and Message Image, pick number of bits that will be taken for hidden message and hit encrypt.
In order to Decrypt: Pick Storage Image, pick correct number of bits and grey or colour scale and hit decrypt, message will be display
ed on the left.

Single click on a image area opens open image dialog, double click save image dialog.
