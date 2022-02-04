#!/usr/bin/env python3
#
# Copyright (c) 2022 The Brave Authors. All rights reserved.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/. */


import requests

from lib.config import get_env_var
from lib.l10n.transifex.api_wrapper import TransifexAPIWrapper
from transifex.api import transifex_api  # pylint: disable=import-error


# This module is a wrapper around Transifex API v3


brave_organization_name = 'brave'
# brave_project_name = 'brave'
brave_project_name = 'maxtest'


class TransifexAPIV3Wrapper(TransifexAPIWrapper):
    """Wrapper class for the Transifex API (v3) python SDK from
       https://github.com/transifex/transifex-python"""
    # pylint: disable=super-init-not-called
    def __init__(self,
                 organization_name = brave_organization_name,
                 project_name = brave_project_name):
        self._organization_name = organization_name
        self._project_name = project_name
        self._transifex_api_key = get_env_var('TRANSIFEX_API_KEY')
        assert self._organization_name, \
            'TransifexAPIWrapper: organization_name is not set.'
        assert self._project_name, \
            'TransifexAPIWrapper: project_name is not set.'
        assert self._transifex_api_key, \
            'TRANSIFEX_API_KEY environmental var is not set.'
        self.auth()


    @property
    def organization_name(self):
        return self._organization_name


    @property
    def project_name(self):
        return self._project_name


    def auth(self):
        """Set up auth to Transifex using an API token. You can generate one at
           https://www.transifex.com/user/settings/api/"""
        print('Authenticating with Transifex...')
        transifex_api.setup(auth=self._transifex_api_key)


    def get_org_obj(self):
        return transifex_api.Organization.get(slug=self._organization_name)


    def get_project_obj(self):
        return self.get_org_obj().fetch('projects').get(slug=self._project_name)


    def get_project_languages(self):
        return self.get_project_obj().fetch('languages')


    # pylint: disable=no-self-use
    def get_lang_obj(self, lang_code):
        return transifex_api.Language.get(code=lang_code)
    # pylint: enable=no-self-use


    def is_supported_language(self, lang_code):
        lang_obj = self.get_lang_obj(lang_code)
        lang_objs = list(self.get_project_obj().fetch('languages'))
        return lang_obj in lang_objs


    def get_resource_obj(self, resource_slug):
        return self.get_project_obj().fetch('resources').get(slug=resource_slug)


    def get_resource_strings_download_url(self, resource_slug):
        resource_obj = self.get_resource_obj(resource_slug)
        return transifex_api.ResourceStringsAsyncDownload.download(
            resource=resource_obj, content_encoding='text', file_type='default')


    def get_resource_l10n_download_url(self, resource_slug, lang_code):
        resource_obj = self.get_resource_obj(resource_slug)
        lang_obj = self.get_lang_obj(lang_code)
        return transifex_api.ResourceTranslationsAsyncDownload.download(
            resource=resource_obj, language=lang_obj)


    def get_i18_format(self, format_name):
        return transifex_api.I18nFormat.filter(organization=self.get_org_obj(),
                                               name=format_name)


    def create_resource(self, resource_slug, i18n_type):
        i18n_format=self.get_i18_format(i18n_type)
        return transifex_api.Resource.create(
            name=resource_slug, slug=resource_slug, i18n_format=i18n_format)


    # pylint: disable=no-self-use
    def upload_resource_content(self, resource_obj, content):
        transifex_api.ResourceStringsAsyncUpload.upload(resource_obj, content)
    # pylint: enable=no-self-use


    def get_resource_string(self, resource_slug, string_key):
        resource_obj = self.get_resource_obj(resource_slug)
        resource_string = transifex_api.ResourceString.get(
            resource=resource_obj, key=string_key)
        return resource_string


    def update_resource_string_instructions(self, resource_slug, string_key,
                                            string_hash, instructions):
        resource_string = self.get_resource_string(resource_slug, string_key)
        assert resource_string.string_hash == string_hash, \
            (f'String with key {string_key} has hash '
             f'{resource_string.string_hash} which is not the expected'
             f'hash {string_hash}')
        resource_string.instructions = instructions
        resource_string.save('instructions')


    """
    def get_resource_translations(self, resource_slug, lang_code):
        resource_obj = self.get_resource_obj(resource_slug)
        lang_obj = self.get_lang_obj(lang_code)
        return transifex_api.ResourceTranslation.filter(resource=resource_obj,
            language=lang_obj).include('resource_string')
    """


    def transifex_get_resource_source(self, resource_name):
        """Downloads resource source strings (original language) from
           Transifex"""
        url = self.get_resource_strings_download_url(resource_name)
        r = requests.get(url)
        assert r.status_code >= 200 and r.status_code <= 299, \
            f'Aborting. Status code {r.status_code}: {r.content}'
        content = r.text.encode('utf-8')
        return content


    def transifex_get_resource_l10n(self, resource_name, lang_code, file_ext):
        """Downloads resource l10n from Transifex for the given language"""
        if not self.is_supported_language(lang_code):
            print (f'WARNING: Language "{lang_code}" has not been added to'
                   f' the "{self.project_name}" project in Transifex')
            # For json files we need to have content even if untranslated, so
            # get the source strings instead
            if file_ext == '.json':
                url = self.get_resource_strings_download_url(resource_name)
            elif file_ext == '.grd':
                # For GRDs we can just return an empty <resources> content:
                return '<resources></resources>'.encode('utf-8')
            else:
                assert False, f'Unexpected file extension {file_ext}'
        else:
            url = self.get_resource_l10n_download_url(resource_name, lang_code)
        r = requests.get(url)
        assert r.status_code >= 200 and r.status_code <= 299, \
            f'Aborting. Status code {r.status_code}: {r.content}'
        content = r.text.encode('utf-8')
        return content


    def transifex_upload_resource_content(self, resource_name, xml_content,
                                          i18n_type):
        """Upload resource content to Transifex"""
        resource_obj = self.get_resource_obj(resource_name)
        if resource_obj:
            print(f'Resource named {resource_name} already exists')
        else:
            print(f'Creating a new resource named {resource_name}')
            resource_obj = self.create_resource(resource_name, i18n_type)

        self.upload_resource_content(resource_obj, xml_content)
        return True


    def transifex_upload_string_desc(self, resource_name, string_key,
                                     string_hash, string_desc):
        """Uploads a description for the string with the given hash"""
        self.update_resource_string_instructions(resource_name, string_key,
                                                 string_hash, string_desc)

    """
    def transifex_upload_string_l10n(self, resource_name, string_hash,
                                     lang_code, translated_value):
        #Uploads the localized string in the given language for the string
        #   with the given hash.
        url_part = (f'project/{transifex_project_name}/resource/{resource_name}'
                    f'/translation/{lang_code}/string/{string_hash}/')
        url = base_url + url_part
        payload = {
            'translation': translated_value,
            # Assume Chromium provided strings are reviewed and proofread
            'reviewed': True,
            'proofread': True,
            'user': 'bbondy'
        }
        headers = {'Content-Type': 'application/json'}
        r = requests.put(url, json=payload, auth=self.get_auth(),
                         headers=headers)
        assert r.status_code >= 200 and r.status_code <= 299, \
            f'Aborting. Status code {r.status_code}: {r.content}'
    """
