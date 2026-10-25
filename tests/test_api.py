#!/usr/bin/env python3
"""Integration tests against the real native process in an isolated profile directory."""
import json
import os
from pathlib import Path
import shutil
import socket
import subprocess
import tempfile
import time
import unittest
import urllib.error
import urllib.parse
import urllib.request
ROOT = Path(__file__).resolve().parents[1]
BINARY = Path(os.environ.get('BW_BINARY', ROOT/'build/boreal-wheel')).resolve()
class BridgeIntegration(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory(prefix='boreal-wheel-test-')
        shutil.copytree(ROOT/'web', Path(cls.tmp.name)/'web')
        with socket.socket() as s:
            s.bind(('127.0.0.1', 0)); cls.port = s.getsockname()[1]
        cls.base = f'http://127.0.0.1:{cls.port}'
        cls.process = subprocess.Popen([str(BINARY), '--demo', '--port', str(cls.port)], cwd=cls.tmp.name, stdout=subprocess.DEVNULL)
        for _ in range(100):
            try: cls.get('/api/state'); break
            except (OSError, ValueError): time.sleep(.03)
        else: raise RuntimeError('Native server did not start')
    @classmethod
    def tearDownClass(cls):
        cls.process.terminate(); cls.process.wait(timeout=5); cls.tmp.cleanup()
    @classmethod
    def get(cls, path):
        with urllib.request.urlopen(cls.base+path, timeout=3) as r: return json.load(r)
    @classmethod
    def post(cls,path,values,origin=True):
        headers={'Content-Type':'application/x-www-form-urlencoded'}
        if origin: headers['Origin']=cls.base
        req=urllib.request.Request(cls.base+'/api/'+path,data=urllib.parse.urlencode(values).encode(),headers=headers)
        with urllib.request.urlopen(req,timeout=3) as r:return json.load(r)
    def wait_state(self,predicate):
        for _ in range(100):
            s=self.get('/api/state')
            if predicate(s):return s
            time.sleep(.01)
        self.fail('Timed out waiting for native state')
    def test_01_demo_is_explicit_and_cannot_output(self):
        s=self.get('/api/state');self.assertEqual(s['demo'],1);self.assertEqual(s['physical_connected'],0)
        with self.assertRaises(urllib.error.HTTPError) as e:self.post('action',{'action':'start'})
        self.assertEqual(e.exception.code,400);e.exception.close();self.assertEqual(s['output_active'],0)
    def test_02_real_mapping_and_simultaneous_pedals(self):
        self.post('action',{'action':'manual'})
        self.post('demo',{'steering':1,'throttle':1,'brake':1,'buttons':4096})
        s=self.wait_state(lambda s:s['report']['lx']==32767)
        self.assertEqual((s['report']['lt'],s['report']['rt']),(255,255))
        self.post('config',{'button_12':13,'deadzone':.1,'gamma':1.5})
        s=self.wait_state(lambda s:s['report']['buttons']==8192)
        self.assertEqual(s['config']['gamma'],1.5)
        self.assertIn('button_source_13=18', (Path(self.tmp.name)/'profile.ini').read_text())
    def test_03_bad_profile_is_atomic(self):
        before=self.get('/api/state')['config']
        for invalid in [{'gamma':'nan'},{'poll_hz':999},{'slot':4},{'deadzone':.2,'unknown':5},{'button_12':11}]:
            with self.assertRaises(urllib.error.HTTPError) as e:self.post('config',invalid)
            self.assertEqual(e.exception.code,400);e.exception.close()
            self.assertEqual(self.get('/api/state')['config'],before)
    def test_04_origin_and_host_restrictions(self):
        with self.assertRaises(urllib.error.HTTPError) as e:self.post('action',{'action':'demo'},origin=False)
        self.assertEqual(e.exception.code,403);e.exception.close()
        req=urllib.request.Request(self.base+'/api/state',headers={'Host':'attacker.invalid'})
        with self.assertRaises(urllib.error.HTTPError) as e:urllib.request.urlopen(req)
        self.assertEqual(e.exception.code,403);e.exception.close()
        with self.assertRaises(urllib.error.HTTPError) as e:urllib.request.urlopen(self.base+'/../profile.ini')
        self.assertEqual(e.exception.code,404);e.exception.close()
    def test_05_disconnect_resets_output(self):
        self.post('action',{'action':'physical'})
        s=self.wait_state(lambda s: not s['demo'] and not s['connected'])
        self.assertEqual(s['report'],dict(lx=0,ly=0,rx=0,ry=0,lt=0,rt=0,buttons=0))
        self.assertEqual(s['physical_connected'],0)
        self.assertEqual(s['output_active'],0)
    def test_06_assets_have_types_and_csp(self):
        for path,typ in [('/','text/html'),('/style.css','text/css'),('/app.js','text/javascript'),('/i18n.js','text/javascript'),('/locales.js','text/javascript'),('/favicon.svg','image/svg+xml')]:
            with urllib.request.urlopen(self.base+path) as r:
                self.assertTrue(r.headers['Content-Type'].startswith(typ)); self.assertIn("frame-ancestors 'none'",r.headers['Content-Security-Policy']);self.assertGreater(len(r.read()),100)
    def test_07_center_validation(self):
        self.post('action',{'action':'demo'});self.post('action',{'action':'manual'})
        self.post('demo',{'steering':1});self.wait_state(lambda s:s['raw']['steering']==1)
        with self.assertRaises(urllib.error.HTTPError) as e:self.post('action',{'action':'center'})
        self.assertEqual(e.exception.code,400);e.exception.close()
        self.post('demo',{'steering':.15});self.wait_state(lambda s:abs(s['raw']['steering']-.15)<.0001)
        self.post('action',{'action':'center'})
        s=self.wait_state(lambda s:s['report']['lx']==0);self.assertAlmostEqual(s['config']['center'],.15,places=5)
    def test_08_reset_and_restart_persist(self):
        self.post('action',{'action':'reset'})
        s=self.get('/api/state');self.assertEqual(s['config']['button_map'][12],12)
        self.assertEqual(s['config']['center'],0)
        self.post('config',{'poll_hz':500,'axis_source_4':5,'axis_source_5':4,'axis_limit_0':.4,'axis_deadzone_5':.03})
        self.process.terminate();self.process.wait(timeout=5)
        self.__class__.process=subprocess.Popen([str(BINARY),'--port',str(self.port)],cwd=self.tmp.name,stdout=subprocess.DEVNULL)
        for _ in range(100):
            try:
                s=self.get('/api/state')
                if s['config']['poll_hz']==500:break
            except OSError:pass
            time.sleep(.02)
        self.assertEqual(s['config']['poll_hz'],500)
        self.assertEqual(s['config']['axis_source_4'],5)
        self.assertEqual(s['config']['axis_source_5'],4)
        self.assertEqual(s['config']['axis_limit_0'],.4)
        self.assertEqual(s['config']['axis_deadzone_5'],.03)
        self.assertEqual(s['demo'],0)
        self.assertEqual(s['output_requested'],0)
    def test_09_full_mapper_and_profile_roundtrip(self):
        self.post('action',{'action':'reset'})
        self.post('action',{'action':'demo'});self.post('action',{'action':'manual'})
        self.post('config',{'axis_source_4':5,'axis_source_5':4,'axis_deadzone_0':.1,'axis_limit_0':.5,'axis_deadzone_4':.1,'axis_limit_4':.9})
        self.post('demo',{'steering':.3,'throttle':0,'brake':.5,'ly':.6,'rx':-.7,'ry':.8})
        s=self.wait_state(lambda s: s['report']['rt']==128)
        self.assertEqual(s['report']['lt'],0)
        self.assertAlmostEqual(s['report']['lx']/32767,.5,places=4)
        self.assertEqual(s['raw']['brake'],.5)
        self.assertAlmostEqual(s['input'][4],.5)
        self.assertGreater(s['report']['ly'],0);self.assertLess(s['report']['rx'],0);self.assertGreater(s['report']['ry'],0)
        self.post('config',{'button_source_12':2,'button_direction_12':-1,'button_threshold_12':.6})
        self.wait_state(lambda s: s['report']['buttons'] & 4096)
        profile=self.get('/api/profile')
        self.assertEqual((profile['format'],profile['version']),('boreal-wheel',2))
        self.assertEqual(len(profile['settings']),87)
        self.post('action',{'action':'reset'})
        self.post('profile',profile['settings'])
        s=self.wait_state(lambda s: s['config']['axis_source_4']==5 and s['report']['rt']==128)
        self.assertEqual(self.get('/api/profile'),profile)
        before=s['config'];saved=(Path(self.tmp.name)/'profile.ini').read_bytes()
        bads=[{'axis_limit_0':.11},{'axis_source_4':28},{'button_threshold_12':0},{'unknown':1}]
        for patch in bads:
            invalid={**profile['settings'],**patch}
            with self.assertRaises(urllib.error.HTTPError) as error:self.post('profile',invalid)
            self.assertEqual(error.exception.code,400);error.exception.close()
            self.assertEqual(self.get('/api/state')['config'],before)
            self.assertEqual((Path(self.tmp.name)/'profile.ini').read_bytes(),saved)
        with self.assertRaises(urllib.error.HTTPError) as error:self.post('profile',{'poll_hz':250})
        error.exception.close()
        # Invalid span cannot be committed even when both fields are submitted together.
        with self.assertRaises(urllib.error.HTTPError) as error:self.post('config',{'axis_deadzone_0':.4,'axis_limit_0':.41})
        error.exception.close()
        self.assertEqual(self.get('/api/state')['config'],before)
    def test_10_native_capture_uses_selected_sources(self):
        self.post('action',{'action':'reset'})
        self.post('action',{'action':'demo'});self.post('action',{'action':'manual'})
        self.post('config',{'axis_source_0':2,'axis_invert_0':1,'axis_source_4':5})
        self.post('demo',{'steering':.4,'rx':.2,'brake':.7})
        self.wait_state(lambda s:s['raw']['rx']==.2)
        self.post('action',{'action':'center_0'})
        self.wait_state(lambda s:abs(s['config']['axis_center_0']+.2)<.0001 and s['report']['lx']==0)
        self.post('action',{'action':'limit_4'})
        self.wait_state(lambda s:abs(s['config']['axis_limit_4']-.7)<.0001 and s['report']['rt']==255)
        before=self.get('/api/state')['config']
        self.post('demo',{});self.wait_state(lambda s:s['raw']['brake']==0)
        with self.assertRaises(urllib.error.HTTPError) as error:self.post('action',{'action':'limit_4'})
        error.exception.close();self.assertEqual(self.get('/api/state')['config'],before)
    def test_11_process_language_and_invalid_language(self):
        result=subprocess.run([str(BINARY),'--lang','invalid'],capture_output=True,text=True,timeout=3)
        self.assertNotEqual(result.returncode,0)
        self.assertIn('Invalid language',result.stderr)
        localized=subprocess.run([str(BINARY),'--port','999','--lang','pt-BR'],capture_output=True,text=True,timeout=3)
        self.assertIn('Porta inválida',localized.stderr)
        override=subprocess.run([str(BINARY),'--port','999','--lang','en'],env={**os.environ,'BW_LANG':'pt-BR'},capture_output=True,text=True,timeout=3)
        self.assertIn('Invalid port',override.stderr)
        with tempfile.TemporaryDirectory(prefix='boreal-wheel-language-') as directory:
            with socket.socket() as sock:sock.bind(('127.0.0.1',0));port=sock.getsockname()[1]
            base=f'http://127.0.0.1:{port}'
            process=subprocess.Popen([str(BINARY),'--demo','--lang','pt-BR','--port',str(port)],cwd=directory,stdout=subprocess.DEVNULL)
            try:
                for _ in range(100):
                    try:
                        with urllib.request.urlopen(base+'/api/state',timeout=1) as response:state=json.load(response)
                        if state['backend']:break
                    except OSError:pass
                    time.sleep(.02)
                self.assertTrue(state['backend'].startswith('Demonstração'))
                self.assertEqual(state['output_status'],'Demonstração · saída para jogos bloqueada')
                req=urllib.request.Request(base+'/api/config',data=b'poll_hz=999',headers={'Origin':base,'Content-Type':'application/x-www-form-urlencoded'})
                with self.assertRaises(urllib.error.HTTPError) as error:urllib.request.urlopen(req)
                self.assertEqual(error.exception.read().decode(),'Ajuste inválido')
                error.exception.close()
            finally:process.terminate();process.wait(timeout=5)
if __name__=='__main__':unittest.main(verbosity=2)
