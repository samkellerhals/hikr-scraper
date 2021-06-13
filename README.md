# hikr-scraper
Scrape tours from hikr.org

#### Description
This is a command-line C application that downloads randomly chosen images from Hikr.org,
which is a non-profit website where people publish reports and photos of their mountain-tours in order to keep memories and share experiences, impressions and other useful information with the community.

This application accepts a positional argument which defines the number of randomly chosen posts to download from Hikr.org. For each downloaded image the HTML page for the corresponding tour is also downloaded. This HTML page is then parsed to identify the title of the tour which is used as a filename for each downloaded image.

##### Structure

The main file of the project is `hikr.c`, this file contains multiple different functions.

THe `main` function contains the main program loop. This function accepts one positional argument,
which is the number of files to download.

This main loop calls a number of other functions, some of which are responsible for generating
random `post ids`, and some of which are responsible for making GET requests to certain URL endpoints.
There is also logic dedicated to parsing HTML.

The `download_file` function uses the `libcurl` library to make GET requests to various Hikr.org endpoints to download both the image as well as the corresponding HTML page of each tour to be downloaded.

libcurl is a free and easy-to-use client-side URL transfer library, supporting DICT, FILE, FTP, FTPS, GOPHER, GOPHERS, HTTP, HTTPS, IMAP, IMAPS, LDAP, LDAPS, MQTT, POP3, POP3S, RTMP, RTMPS, RTSP, SCP, SFTP, SMB, SMBS, SMTP, SMTPS, TELNET and TFTP. libcurl supports SSL certificates, HTTP POST, HTTP PUT, FTP uploading, HTTP form based upload, proxies, HTTP/2, HTTP/3, cookies, user+password authentication (Basic, Digest, NTLM, Negotiate, Kerberos), file transfer resume, http proxy tunneling and more!

libcurl is free, thread-safe, IPv6 compatible, feature rich, well supported, fast, thoroughly documented and is already used by many known, big and successful companies.

The title is parsed out in the following way. The `extractLocationsFromHtml` function determines the number of bytes stored inside the HTML file, and determines the indexes at which we find the first instance of the `div13` class, which contains all information relevant to the post location inside each Hikr tour. A slice is created, and passed to the `getLinkContent` function which ultimately extracts the inner HTML content of the first link (which corresponds to the tour title). This text is then used as the file name for the downloaded image.

The information for each post is saved inside a `Location` struct. In the future it is planned to extend this in the following ways:

- Save geographical information of each post, such as region, and country.
- Save downloaded HTML files in a temporary folder.
- Generate a PDF report using the downloaded images and metadata.

## Installation

This program can be compiled by linking against the `libcurl` library.

`gcc hikr.c -o hikr_scraper -lcurl`

## Usage

`./hikr_scraper <number-of-images-to-download>` 

**Note**: Keep the number of images small or Hikr.org will block your requests.

The images will be saved in the `img` folder.