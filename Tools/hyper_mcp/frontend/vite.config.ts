import {defineConfig} from 'vite';
import react from '@vitejs/plugin-react';

// The Python hyper_router serves the built SPA from ./dist and exposes the
// JSON API + agent endpoints. During `npm run dev` we run Vite separately and
// proxy those paths to a locally running router so the console works identically
// in dev and production. Point it at your router if you run it elsewhere.
const BACKEND = 'http://127.0.0.1:8765';

export default defineConfig({
  plugins: [react()],
  build: {
    outDir: 'dist',
    emptyOutDir: true,
  },
  server: {
    host: true,
    port: 5173,
    proxy: {
      '/api': {target: BACKEND, changeOrigin: true},
      '/agent': {target: BACKEND, changeOrigin: true},
      '/health': {target: BACKEND, changeOrigin: true},
    },
  },
});
